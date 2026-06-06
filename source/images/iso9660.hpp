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

namespace firy {
    namespace images {
        namespace iso9660 {

            // ECMA-119 fixed sector size for the data area of CD-ROM images.
            // Some niche CD/CDR images use 2336/2352 raw sectors (with sync +
            // header + ECC); for the engine's purposes we only support the
            // cooked 2048-byte form, which is what every Cannon Fodder DOS-CD
            // / GOG-CD ISO ships as.
            constexpr size_t kSectorSize = 2048;

            // Sector 16 is the start of the Volume Descriptor Set.
            constexpr size_t kVolumeDescriptorStartSector = 16;

            // Volume Descriptor Type codes (ECMA-119 §8.1.1).
            enum eVolumeDescriptorType : uint8_t {
                eVD_BootRecord                = 0,
                eVD_Primary                   = 1,
                eVD_Supplementary             = 2,    // Joliet uses this for UCS-2 names
                eVD_VolumePartition           = 3,
                eVD_Terminator                = 255,
            };

            // File-flags bits inside a Directory Record (ECMA-119 §9.1.6).
            enum eFileFlags : uint8_t {
                eFF_Hidden       = 0x01,
                eFF_Directory    = 0x02,
                eFF_AssociatedFile = 0x04,
                eFF_Record       = 0x08,
                eFF_Protection   = 0x10,
                eFF_MultiExtent  = 0x80,
            };

            // ECMA-119 stores most multi-byte fields twice — once little-
            // endian and once big-endian, back to back. We only ever read the
            // LE half and ignore the BE one.
            struct sBoth16 {
                uint16_t mLE;
                uint16_t mBE;
            };
            struct sBoth32 {
                uint32_t mLE;
                uint32_t mBE;
            };

#pragma pack(push, 1)
            // Directory Record on disk (ECMA-119 §9.1). Variable length
            // because of the trailing FileIdentifier; total length is given
            // by mLengthOfRecord.
            struct sDirectoryRecord {
                uint8_t  mLengthOfRecord;          // Length of this entry in bytes (rounded up to even)
                uint8_t  mExtAttrLength;           // Length of extended attribute record
                sBoth32  mExtentLocation;          // LBA of the file's first sector
                sBoth32  mExtentSize;              // Size in bytes
                uint8_t  mDateTime[7];             // y,m,d,h,m,s,gmt-offset (15-min units)
                uint8_t  mFileFlags;
                uint8_t  mFileUnitSize;            // Interleaved files only
                uint8_t  mInterleaveGap;
                sBoth16  mVolumeSeqNumber;
                uint8_t  mFileIdentifierLength;
                // FileIdentifier follows here; then padding byte (only if
                // FileIdentifierLength is even) so the next record is 16-bit
                // aligned, then optional system-use area.
            };

            // Primary / Supplementary Volume Descriptor (ECMA-119 §8.4 and
            // Joliet specification §6.4). Layout is identical between PVD
            // and SVD; the only differences are the encoding of strings
            // (a-characters in PVD, UCS-2 in Joliet SVD) and the Escape
            // Sequences field at offset 88.
            struct sVolumeDescriptor {
                uint8_t  mType;                    // 1 = PVD, 2 = SVD, 255 = terminator
                char     mStandardIdentifier[5];   // "CD001"
                uint8_t  mVersion;                 // 1
                uint8_t  mFlags;                   // PVD: unused (0); SVD: bit 0 = "not registered"
                char     mSystemIdentifier[32];
                char     mVolumeIdentifier[32];
                uint8_t  mUnused1[8];
                sBoth32  mVolumeSpaceSize;         // Total sectors in volume
                uint8_t  mEscapeSequences[32];     // Joliet level encoded here for SVD; zeros for PVD
                sBoth16  mVolumeSetSize;
                sBoth16  mVolumeSeqNumber;
                sBoth16  mLogicalBlockSize;        // 2048 in practice
                sBoth32  mPathTableSize;
                uint32_t mTypeLPathTableLBA;
                uint32_t mOptTypeLPathTableLBA;
                uint32_t mTypeMPathTableLBA;
                uint32_t mOptTypeMPathTableLBA;
                uint8_t  mRootDirectoryRecord[34]; // Embedded sDirectoryRecord
                char     mVolumeSetIdentifier[128];
                char     mPublisherIdentifier[128];
                char     mDataPreparerIdentifier[128];
                char     mApplicationIdentifier[128];
                char     mCopyrightFileIdentifier[37];
                char     mAbstractFileIdentifier[37];
                char     mBibliographicFileIdentifier[37];
                uint8_t  mDateTimeVolCreate[17];
                uint8_t  mDateTimeVolModify[17];
                uint8_t  mDateTimeVolExpire[17];
                uint8_t  mDateTimeVolEffective[17];
                uint8_t  mFileStructureVersion;    // 1
                uint8_t  mUnused3;
                uint8_t  mApplicationUse[512];
                uint8_t  mReserved[653];
            };
            static_assert(sizeof(sVolumeDescriptor) == 2048, "ISO9660 VD must be exactly one sector");
#pragma pack(pop)

            // Per-node bookkeeping the engine needs at read time. Identifiers
            // come from either the PVD (a-characters; we strip the ";1" version
            // suffix) or the Joliet SVD (UCS-2BE; we transcode to UTF-8).
            struct sEntry {
                uint32_t mExtentLBA = 0;       // First sector of the file/dir
                uint32_t mExtentSize = 0;      // Bytes (data fork)
            };

            struct sFile : public sEntry, public filesystem::sFile {
                sFile(wpFilesystem pFilesystem, const std::string& pName = "")
                    : filesystem::sFile(pFilesystem, pName) {}
            };

            struct sDir : public sEntry, public filesystem::sDirectory {
                sDir(wpFilesystem pFilesystem, const std::string& pName = "")
                    : filesystem::sDirectory(pFilesystem, pName) {}
            };

            using spFile = std::shared_ptr<sFile>;
            using spDir  = std::shared_ptr<sDir>;

        } // namespace iso9660

        /**
         * ISO9660 / ECMA-119 CD-ROM filesystem (read-only).
         *
         * Supports Primary Volume Descriptor with optional Joliet (UCS-2)
         * Supplementary Volume Descriptor for long/Unicode filenames. Rock
         * Ridge is NOT decoded — Cannon Fodder's PC retail and GOG ISOs
         * don't use it, and our use case (asset extraction) doesn't need
         * POSIX names.
         */
        class cISO9660 : public cImageAccess<access::cBlocks> {
        public:
            cISO9660(spSource pSource);

            virtual std::string imageTypeShort() const override { return "iso"; }
            virtual std::string imageType() const override {
                return "ISO 9660 / ECMA-119 (iso)";
            }

            static std::vector<std::string> imageExtensions() {
                return { "iso" };
            }
            static bool imageTest(spSource pSource);

            // --- Read-only filesystem interface -----------------------------
            virtual bool filesystemCreate() override { return false; }
            virtual bool filesystemLoad() override;
            virtual spBuffer filesystemRead(spNode pFile) override;
            virtual bool filesystemRemove(spNode /*pFile*/) override { return false; }

            virtual spFile filesystemFileCreate(const std::string& pName = "") override {
                return filesystemNodeCreate<iso9660::sFile>(pName);
            }
            virtual spDirectory filesystemDirectoryCreate(const std::string& pName = "") override {
                return filesystemNodeCreate<iso9660::sDir>(pName);
            }

            virtual size_t filesystemTotalBytesFree() override { return 0; }
            virtual size_t filesystemTotalBytesMax() override;

            // --- cBlocks: stubbed read-only ---------------------------------
            virtual size_t blockSize(const tBlock /*pBlock*/ = 0) const override { return iso9660::kSectorSize; }
            virtual bool   blockIsFree(const tBlock /*pBlock*/) const override   { return false; }
            virtual bool   blockSet(const tBlock /*pBlock*/, const bool /*pValue*/) override { return false; }

            virtual std::vector<sAccessUnit> blockUse(const tBlock /*pTotal*/) override   { return {}; }
            virtual bool blocksFree(const std::vector<sAccessUnit>& /*pBlocks*/) override { return false; }
            virtual std::vector<sAccessUnit> blocksGetFree() const override                { return {}; }

        protected:
            virtual bool filesystemChainLoad(spFile /*pFile*/) override        { return true; }
            virtual bool filesystemBitmapLoad() override                        { return true; }
            virtual bool filesystemBitmapSave() override                        { return false; }
            virtual bool filesystemDirectoryLoad(spDirectory pDir) override;
            virtual bool filesystemSaveNative() override                        { return false; }

        private:
            // Walk the Volume Descriptor Set at sector 16+. Picks the PVD as
            // the main view and remembers whether a usable Joliet SVD exists.
            bool readVolumeDescriptors();

            // Parse a single 2048-byte directory extent into pDir. Handles
            // multi-extent directories by following mExtentSize across sectors.
            bool parseDirectoryExtent(iso9660::spDir pDir,
                                       uint32_t pLBA,
                                       uint32_t pSizeBytes);

            // Decode an iso9660 file identifier from its on-disk
            // representation. Strips the ";N" version suffix that retail CD
            // masters always carry, and trailing dots. If mIsJoliet is set
            // and pJolietSource is non-null, transcodes UCS-2BE → UTF-8.
            std::string decodeIdentifier(const uint8_t* pData,
                                          uint8_t pLength,
                                          bool pUseJoliet) const;

        private:
            // Volume metadata read from the chosen descriptor.
            uint32_t mRootExtentLBA = 0;
            uint32_t mRootExtentSize = 0;
            uint64_t mVolumeSpaceBytes = 0;

            // True iff we ended up using a Joliet Supplementary Volume
            // Descriptor instead of the Primary one, i.e. filenames are UCS-2.
            bool mIsJoliet = false;
        };

    } // namespace images
} // namespace firy
