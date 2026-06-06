/*
 *  FIRY
 *  ---------------
 *
 *  Copyright (C) 2019-2026
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "firy.hpp"
#include "images/iso9660.hpp"

#include <algorithm>
#include <cstring>

namespace firy {
    namespace images {

        // -------- helpers ---------------------------------------------------

        // Joliet escape sequences (ECMA-119 §B.1, Joliet spec §6.7.1) — three
        // recognised level encodings. Any of these in the SVD's
        // mEscapeSequences identifies it as Joliet UCS-2BE.
        static bool isJolietEscape(const uint8_t* pEsc) {
            //  Level 1: 25 2F 40   ('%','/','@')
            //  Level 2: 25 2F 43   ('%','/','C')
            //  Level 3: 25 2F 45   ('%','/','E')
            if (pEsc[0] != '%' || pEsc[1] != '/')
                return false;
            return (pEsc[2] == '@' || pEsc[2] == 'C' || pEsc[2] == 'E');
        }

        // Strip the ECMA-119 ";1" version suffix that retail CD masters
        // always carry on file identifiers, plus any trailing dots.
        static void stripVersionAndDots(std::string& pName) {
            auto semi = pName.rfind(';');
            if (semi != std::string::npos)
                pName.resize(semi);
            while (!pName.empty() && pName.back() == '.')
                pName.pop_back();
        }

        // Decode UCS-2 big-endian (Joliet) into UTF-8. We deliberately skip
        // surrogate-pair handling — Joliet uses BMP only.
        static std::string ucs2beToUtf8(const uint8_t* pData, size_t pBytes) {
            std::string out;
            out.reserve(pBytes);
            for (size_t i = 0; i + 1 < pBytes; i += 2) {
                uint32_t cp = ((uint32_t)pData[i] << 8) | pData[i + 1];
                if (cp == 0)
                    break;
                if (cp < 0x80) {
                    out.push_back((char)cp);
                } else if (cp < 0x800) {
                    out.push_back((char)(0xC0 | (cp >> 6)));
                    out.push_back((char)(0x80 | (cp & 0x3F)));
                } else {
                    out.push_back((char)(0xE0 | (cp >> 12)));
                    out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
                    out.push_back((char)(0x80 | (cp & 0x3F)));
                }
            }
            return out;
        }

        // -------- detection -------------------------------------------------

        bool cISO9660::imageTest(spSource pSource) {
            if (!pSource)
                return false;

            const auto sz = pSource->size();
            const auto neededOffset = iso9660::kVolumeDescriptorStartSector * iso9660::kSectorSize;
            if (sz < neededOffset + iso9660::kSectorSize)
                return false;

            // Read just the 7-byte VD header at sector 16. If it's a PVD or
            // any standard volume descriptor it carries the magic
            // "CD001" identifier at offset 1.
            auto buf = pSource->chunkCopyToBuffer(neededOffset, 8);
            if (!buf || buf->size() < 7)
                return false;

            // Type byte must be a known VD type code.
            const uint8_t type = buf->at(0);
            if (type != iso9660::eVD_BootRecord &&
                type != iso9660::eVD_Primary &&
                type != iso9660::eVD_Supplementary &&
                type != iso9660::eVD_VolumePartition &&
                type != iso9660::eVD_Terminator)
                return false;

            // "CD001" magic identifier.
            return buf->at(1) == 'C' && buf->at(2) == 'D' &&
                   buf->at(3) == '0' && buf->at(4) == '0' && buf->at(5) == '1';
        }

        // -------- ctor / load ----------------------------------------------

        cISO9660::cISO9660(spSource pSource)
            : cImageAccess<access::cBlocks>(),
              access::cInterface(pSource) {
            // CD-ROM cooked-mode sectors are always 2048 bytes.
            mBlockSize = iso9660::kSectorSize;
            mBlockCount = 0;
        }

        bool cISO9660::filesystemLoad() {
            if (!readVolumeDescriptors())
                return false;

            if (mRootExtentSize == 0)
                return false;

            mBlockCount = (tBlock)(mVolumeSpaceBytes / iso9660::kSectorSize);
            if (mBlockCount == 0 && mSource)
                mBlockCount = (tBlock)(mSource->size() / iso9660::kSectorSize);

            // Build the root directory node. Lazy-load its contents the first
            // time anyone walks it via filesystemNode/filesystemPath.
            auto root = std::make_shared<iso9660::sDir>(weak_from_this(), "/");
            root->mExtentLBA = mRootExtentLBA;
            root->mExtentSize = mRootExtentSize;
            root->entriesLoadedSet(false);
            mFsRoot = root;

            return true;
        }

        size_t cISO9660::filesystemTotalBytesMax() {
            return (size_t)mVolumeSpaceBytes;
        }

        // -------- volume-descriptor scan -----------------------------------

        bool cISO9660::readVolumeDescriptors() {
            mIsJoliet = false;
            mRootExtentLBA = 0;
            mRootExtentSize = 0;
            mVolumeSpaceBytes = 0;

            // Cap the scan so a corrupt image can't loop us forever — the
            // ECMA-119 spec doesn't fix a count, but in practice a sane
            // image has <10 descriptors before the terminator.
            const size_t kMaxDescriptors = 64;

            std::shared_ptr<iso9660::sVolumeDescriptor> pvd;
            std::shared_ptr<iso9660::sVolumeDescriptor> svd;

            for (size_t i = 0; i < kMaxDescriptors; ++i) {
                const size_t lba = iso9660::kVolumeDescriptorStartSector + i;
                const size_t off = lba * iso9660::kSectorSize;

                if (mSource->size() < off + sizeof(iso9660::sVolumeDescriptor))
                    break;

                auto vd = sourceObjectGet<iso9660::sVolumeDescriptor>(off);
                if (!vd)
                    break;

                // CD001 magic — bail if missing, but only on the first
                // descriptor (later sectors might be unrelated trailing data
                // on a hybrid disc image).
                if (std::memcmp(vd->mStandardIdentifier, "CD001", 5) != 0) {
                    if (i == 0)
                        return false;
                    break;
                }

                if (vd->mType == iso9660::eVD_Terminator)
                    break;

                if (vd->mType == iso9660::eVD_Primary) {
                    pvd = vd;
                    continue;
                }

                if (vd->mType == iso9660::eVD_Supplementary) {
                    if (isJolietEscape(vd->mEscapeSequences)) {
                        // Prefer Joliet if multiple SVDs exist (rare).
                        if (!svd)
                            svd = vd;
                    }
                    continue;
                }

                // Boot records and other descriptor types: ignore.
            }

            if (!pvd)
                return false;

            // Choose the descriptor we'll read directory tree from.
            const iso9660::sVolumeDescriptor* chosen = pvd.get();
            if (svd) {
                chosen = svd.get();
                mIsJoliet = true;
            }

            // Volume-space size is in sectors — convert to bytes.
            mVolumeSpaceBytes = (uint64_t)chosen->mVolumeSpaceSize.mLE * iso9660::kSectorSize;

            // Root directory record is embedded directly in the descriptor at
            // offset 156. Decode its extent location + size.
            const auto* root = reinterpret_cast<const iso9660::sDirectoryRecord*>(chosen->mRootDirectoryRecord);
            mRootExtentLBA = root->mExtentLocation.mLE;
            mRootExtentSize = root->mExtentSize.mLE;

            // Volume label — useful for diagnostics. Strip trailing spaces.
            std::string label(chosen->mVolumeIdentifier, sizeof(chosen->mVolumeIdentifier));
            // PVD strings are space-padded; Joliet are UCS-2BE space-padded.
            if (mIsJoliet) {
                label = ucs2beToUtf8(reinterpret_cast<const uint8_t*>(chosen->mVolumeIdentifier),
                                      sizeof(chosen->mVolumeIdentifier));
            }
            while (!label.empty() && (label.back() == ' ' || label.back() == '\0'))
                label.pop_back();
            filesystemNameSet(label);

            return true;
        }

        // -------- directory load --------------------------------------------

        bool cISO9660::filesystemDirectoryLoad(spDirectory pDir) {
            auto dir = std::dynamic_pointer_cast<iso9660::sDir>(pDir);
            if (!dir)
                return false;

            return parseDirectoryExtent(dir, dir->mExtentLBA, dir->mExtentSize);
        }

        bool cISO9660::parseDirectoryExtent(iso9660::spDir pDir,
                                              uint32_t pLBA,
                                              uint32_t pSizeBytes) {
            if (pLBA == 0 || pSizeBytes == 0)
                return false;

            pDir->mNodes.clear();
            pDir->entriesLoadedSet(false);

            auto buffer = sourceBufferCopy((size_t)pLBA * iso9660::kSectorSize, pSizeBytes);
            if (!buffer || buffer->size() < pSizeBytes)
                return false;

            // ECMA-119 §6.8.1.1: directory records do not span sectors. A
            // record whose length is 0 means "rest of this sector is padding,
            // skip to the next 2 KiB boundary." Walk sector-by-sector to keep
            // the alignment rule simple and obvious.
            for (uint32_t sectorOff = 0; sectorOff < pSizeBytes; sectorOff += (uint32_t)iso9660::kSectorSize) {
                uint32_t pos = sectorOff;
                // (std::min) parenthesised to dodge the Windows.h `min` macro
                // which is pulled in transitively through firy.hpp on MSVC.
                const uint32_t sectorEnd = (std::min)(sectorOff + (uint32_t)iso9660::kSectorSize, pSizeBytes);

                while (pos + sizeof(iso9660::sDirectoryRecord) <= sectorEnd) {
                    const uint8_t recLen = buffer->at(pos);
                    if (recLen == 0)
                        break;     // pad to end of sector

                    if (pos + recLen > sectorEnd)
                        break;     // malformed — record claims to span sectors

                    const auto* rec = reinterpret_cast<const iso9660::sDirectoryRecord*>(buffer->data() + pos);
                    const uint8_t idLen = rec->mFileIdentifierLength;

                    // Reject obviously corrupt records.
                    if ((size_t)recLen < sizeof(iso9660::sDirectoryRecord) + idLen) {
                        pos += recLen;
                        continue;
                    }

                    const uint8_t* idBytes =
                        reinterpret_cast<const uint8_t*>(rec) + sizeof(iso9660::sDirectoryRecord);

                    // Skip the "." (single 0x00 byte name) and ".." (single
                    // 0x01 byte name) self/parent pseudo-entries.
                    bool isDotOrDotDot = (idLen == 1 && (idBytes[0] == 0x00 || idBytes[0] == 0x01));
                    if (isDotOrDotDot) {
                        pos += recLen;
                        continue;
                    }

                    // Skip associated files (rarely used; not interesting
                    // for our retail-data scan). Skip multi-extent files we
                    // can't reassemble without chasing the chain — none of
                    // Cannon Fodder's data DAT/RAW files are >2 GiB.
                    if (rec->mFileFlags & iso9660::eFF_AssociatedFile) {
                        pos += recLen;
                        continue;
                    }

                    std::string name = decodeIdentifier(idBytes, idLen, mIsJoliet);
                    if (name.empty()) {
                        pos += recLen;
                        continue;
                    }

                    const bool isDir = (rec->mFileFlags & iso9660::eFF_Directory) != 0;

                    if (isDir) {
                        // adf.cpp uses weak_from_this() (inherited from cImage)
                        // when building child filesystem nodes — same here.
                        auto childDir = std::make_shared<iso9660::sDir>(weak_from_this(), name);
                        childDir->mExtentLBA = rec->mExtentLocation.mLE;
                        childDir->mExtentSize = rec->mExtentSize.mLE;
                        childDir->entriesLoadedSet(false);
                        pDir->nodeAdd(childDir);
                    } else {
                        auto childFile = std::make_shared<iso9660::sFile>(weak_from_this(), name);
                        childFile->mExtentLBA = rec->mExtentLocation.mLE;
                        childFile->mExtentSize = rec->mExtentSize.mLE;
                        childFile->sizeInBytesSet(rec->mExtentSize.mLE);
                        pDir->nodeAdd(childFile);
                    }

                    pos += recLen;
                }
            }

            pDir->entriesLoadedSet(true);
            return true;
        }

        std::string cISO9660::decodeIdentifier(const uint8_t* pData,
                                                  uint8_t pLength,
                                                  bool pUseJoliet) const {
            std::string out;

            if (pUseJoliet) {
                out = ucs2beToUtf8(pData, pLength);
            } else {
                // PVD identifiers are d-characters (A-Z, 0-9, '_') plus the
                // separators '.' and ';'. Lower-case is invalid in strict
                // ISO9660, but ImgBurn etc. emit it anyway — pass through
                // verbatim and let case-insensitive lookups handle it.
                out.assign(reinterpret_cast<const char*>(pData), pLength);
            }

            stripVersionAndDots(out);
            return out;
        }

        // -------- read ------------------------------------------------------

        spBuffer cISO9660::filesystemRead(spNode pFile) {
            auto file = std::dynamic_pointer_cast<iso9660::sFile>(pFile);
            if (!file)
                return {};

            if (file->mExtentSize == 0)
                return std::make_shared<tBuffer>();    // empty file

            // ISO9660 files are contiguous extents — one read does it.
            return sourceBufferCopy((size_t)file->mExtentLBA * iso9660::kSectorSize,
                                     file->mExtentSize);
        }

    } // namespace images
} // namespace firy
