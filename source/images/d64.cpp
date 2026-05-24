/*
 *  FIRY
 *  ---------------
 *
 *  Copyright (C) 2019-2021
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
#include "d64.hpp"
#include <math.h>

namespace firy {
	namespace images {
		namespace d64 {
			/**
			 * Maximum number of bytes which can be stored in a block
			 */
			const size_t gDataBytesPerSector = 254;

			bool isTrackValidForImage(const d64::eType pType, const tTrack pTrack) {
				if (pTrack <= 0)
					return false;

					switch (pType) {
					case d64::eType_D71:
						return pTrack <= 70;
					case d64::eType_D81:
						return pTrack <= 80;
					case d64::eType_D64:
						return pTrack <= 35;
					default:
						return false;
					}
			}

			/**
			 * D64File Constructor
			 */
			sFile::sFile(wpFilesystem pFilesystem, const std::string& pName) : filesystem::sFile(pFilesystem, pName) {
				mType = eFileType_PRG;
				mFlags = eFileFlag_CLOSED;
				mSizeInSectors = 0;
			}
		}

		/**
		 * D64 Constructor
		 */
		cD64::cD64(spSource pSource) : cImageAccess<access::cTracks>(), access::cInterface(pSource) {

			mImageType = d64::eType_Unknown;
			mTrackCount = 35;
			mDosVersion = 0x21;
			mDiskID = 0;
			mDosType = 0;
			mDirectoryTrack = 18;
			mDirectorySector = 1;
			mBAMSectors = { {18, 0} };

			imageTypeDetect();
		}

		/**
		 * Test if this source is a D64/D71/D81 image
		 */
		bool cD64::imageTest(spSource pSource) {
			if (!pSource)
				return false;

			const auto size = pSource->size();
			if (size == 1366 * 256 ||
				size == 80 * 40 * 256 ||
				size == 683 * 256)
				return true;

			return false;
		}

		/**
		 * Configure image geometry and metadata from image size
		 */
		void cD64::imageTypeDetect() {
			const auto size = sourceSize();
			auto type = d64::eType_Unknown;

			if (size == imageCapacity(d64::eType_D71))
				type = d64::eType_D71;
			else if (size == imageCapacity(d64::eType_D81))
				type = d64::eType_D81;
			else if (size == imageCapacity(d64::eType_D64))
				type = d64::eType_D64;

			mImageType = type;
			switch (type) {
			case d64::eType_D81:
				mTrackCount = 80;
				mDirectoryTrack = 40;
				mDirectorySector = 3;
				mBAMSectors = { {40, 1}, {40, 2} };
				mDosVersion = 'D';
				mDosType = '3D';
				break;

			case d64::eType_D71:
				mTrackCount = 70;
				mDirectoryTrack = 18;
				mDirectorySector = 1;
				mBAMSectors = { {18, 0}, {53, 0} };
				mDosVersion = 'A';
				mDosType = '2A';
				break;

			case d64::eType_D64:
			mTrackCount = 35;
				mDirectoryTrack = 18;
				mDirectorySector = 1;
				mBAMSectors = { {18, 0} };
				mDosVersion = 'A';
				mDosType = '2A';
				break;

			default:
				mTrackCount = 0;
				mDirectoryTrack = 0;
				mDirectorySector = 0;
				mBAMSectors.clear();
				mDosVersion = 0;
				mDosType = 0;
				break;
			}
		}

		/**
		 * Size in bytes for each format
		 */
		size_t cD64::imageCapacity(const d64::eType pType) const {
			switch (pType) {
			case d64::eType_D71:
				return 1366 * 256;
			case d64::eType_D81:
				return 80 * 40 * 256;
			case d64::eType_D64:
			default:
				return 683 * 256;
			}
		}

		/**
		 * Map a physical track number to a 1541-style zone for per-track sector count lookups
		 */
		tTrack cD64::sectorToSideTrack(const tTrack pTrack) const {
			if (mImageType == d64::eType_D71 && pTrack > 35)
				return (pTrack - 35);
			return pTrack;
		}

		/**
		 * Track sector count based on image geometry
		 */
		size_t cD64::sectorsPerTrack(const tTrack pTrack) const {
			if (mImageType == d64::eType_D81) {
				if (pTrack > 80 || pTrack <= 0)
					return 0;
				return 40;
			}

			const auto track = sectorToSideTrack(pTrack);
			if (!d64::isTrackValidForImage(mImageType, track))
				return 0;

			return ((21 - (track > 17) * 2) - (track > 24) - (track > 30));
		}

		/**
		 * Load a BAM entry set from a BAM sector
		 */
		bool cD64::loadBamBlock(const tTrackSector pTS, const tTrack pStartTrack, const tTrack pEndTrack, const uint8_t pBytesPerTrackBam, const tSector pBamOffset, tTrack& pTrackCursor) {
			auto block = sectorRead(pTS);
			if (!block)
				return false;

			pTrackCursor = pStartTrack;
			for (tSector index = 0; index < pEndTrack - pStartTrack + 1; ++index) {
				tSector offset = pBamOffset + (index * pBytesPerTrackBam);
				if (offset + pBytesPerTrackBam > sectorSize())
					return false;
				if (pTrackCursor > trackCount())
					break;

				d64::sTrackBam bam;
				bam.mFreeSectors = block->getByte(offset);
				bam.mSectors = 0;
				for (size_t b = 0; b < pBytesPerTrackBam - 1; ++b) {
					bam.mSectors |= (uint64_t)block->getByte(offset + 1 + b) << (8 * b);
				}
				mBam.push_back(bam);
				++pTrackCursor;
			}

			return true;
		}

		/**
		 * Save a BAM entry set to a BAM sector
		 */
		bool cD64::saveBamBlock(const tTrackSector pTS, const tTrack pStartTrack, const tTrack pEndTrack, const uint8_t pBytesPerTrackBam, const tSector pBamOffset, tTrack& pTrackCursor) {
			auto block = sectorRead(pTS);
			if (!block)
				return false;

			pTrackCursor = pStartTrack;
			for (tSector index = 0; index < pEndTrack - pStartTrack + 1; ++index) {
				tSector offset = pBamOffset + (index * pBytesPerTrackBam);
				if (offset + pBytesPerTrackBam > sectorSize())
					return false;
				if (pTrackCursor > trackCount())
					break;

				const auto& bam = mBam.at(pTrackCursor - 1);
				block->putByte(offset, (uint8_t)bam.mFreeSectors);
				for (size_t b = 0; b < pBytesPerTrackBam - 1; ++b) {
					block->putByte(offset + 1 + b, (uint8_t)(bam.mSectors >> (8 * b)));
				}
				++pTrackCursor;
			}

			return sectorWrite(pTS, block);
		}

		/**
		 * Create an empty filesystem
		 */
		bool cD64::filesystemCreate() {
			mFsRoot = std::make_shared<firy::filesystem::sDirectory>(weak_from_this(), "/");

			if (sourceSize() == 0) {
				sourceChunkPrepare(imageCapacity(d64::eType_D64));
				mImageType = d64::eType_D64;
				imageTypeDetect();
			}
			else {
				imageTypeDetect();
				if (mImageType == d64::eType_Unknown)
					return false;
			}

			mDosVersion = 0x21;
			mDiskID = 'FI';
			mBam.assign(trackCount(), d64::sTrackBam());

			tTrackSector ts = { 1, 0 };
			for (; ts.first <= trackCount(); ++ts.first) {
				for (ts.second = 0; ts.second < sectorCount(ts.first); ++ts.second) {
					sectorSet(ts, false);
				}
			}

			sectorSet({ mDirectoryTrack, mDirectorySector }, true);
			for (auto& sector : mBAMSectors) {
				sectorSet(sector, true);
			}

			return filesystemBitmapSave();
		}

		/**
		 * Validate a chain pointer for this image geometry
		 */
		bool cD64::trackSectorValid(const tTrackSector& pTS) const {
			if (pTS.first <= 0 || !d64::isTrackValidForImage(mImageType, pTS.first))
				return false;
			if (pTS.second >= sectorCount(pTS.first))
				return false;
			return true;
		}

		/**
		 * Load the D64 directory
		 */
		bool cD64::filesystemLoad() {
			if (mImageType == d64::eType_Unknown)
				return false;

			mFsRoot = std::make_shared<filesystem::sDirectory>(weak_from_this(), "/");
			std::vector<tTrackSector> visited;

			// Loop until we reach the end of the directory
			tTrackSector ts(mDirectoryTrack, mDirectorySector);
			while ((ts.first > 0 && ts.first <= trackCount()) &&
				(ts.second < sectorCount(ts.first))) {
				if (std::find(visited.begin(), visited.end(), ts) != visited.end()) {
					error("Directory chain loop detected");
					break;
				}
				visited.push_back(ts);

				auto sectorBuffer = sectorRead(ts);
				if (!sectorBuffer)
					return false;

				// 8 entries per sector, 0x20 bytes per entry
				for (size_t i = 0; i <= 7; ++i) {
					d64::spFile file = filesystemEntryLoad(sectorBuffer, i * 0x20);
					if (file) {
						file->mDirIndex.mTS = ts;
						file->mDirIndex.mOffset = i * 20;
						mFsRoot->nodeAdd(file);
					}
				}

				// Get the next Track/Sector in the chain
				tTrackSector nextTs = { sectorBuffer->getByte(0), sectorBuffer->getByte(1)};
				if (nextTs.first == 0 ||
					nextTs.second == 0 ||
					nextTs == ts ||
					!trackSectorValid(nextTs))
					break;
				ts = nextTs;
			}
			return filesystemBitmapLoad();
		}

		/**
		 * Load a file off the D64
		 */
		spBuffer cD64::filesystemRead(spNode pNode) {
			d64::spFile File = std::dynamic_pointer_cast<d64::sFile>(pNode);
			if (!File)
				return 0;

			if (!filesystemChainLoad(File))
				return 0;
			
			// Prepare a buffer to hold the file
			spBuffer buffer = std::make_shared<tBuffer>();
			for (auto& ts : File->mChain) {
				auto sector = sectorRead(ts.mTS);
				if (!sector) {
					File->mChainBroken = true;
					break;
				}

				size_t size = d64::gDataBytesPerSector;
				// If track is zero, this is the last sector
				if (!sector->getByte(0)) {
					const auto bytesUsed = sector->getByte(1);
					if (bytesUsed == 0) {
						File->mChainBroken = true;
						break;
					}
					size = bytesUsed - 1;
				}

				if (!buffer->pushBuffer(sector, 2, size)) {
					File->mChainBroken = true;
					break;
				}
			}

			return buffer;
		}

		/**
		 * Remove a node from the filesystem
		 */
		bool cD64::filesystemRemove(spNode pNode) {
			auto file = std::dynamic_pointer_cast<d64::sFile>(pNode);

			file->dirty();
			file->mType = d64::eFileType_DEL;
			file->sizeInBytesSet(0);
			sectorsFree(file->mChain);
			file->mChain.clear();
			file->remove();
			return true;
		}

		/**
		 * Find a free space in the directory index
		 */
		d64::spFile cD64::filesystemFindFreeIndex(d64::spFile pFile) {
			tTrackSector ts(mDirectoryTrack, mDirectorySector);
			std::vector<tTrackSector> visited;

			while ((ts.first > 0 && ts.first <= trackCount()) &&
				(ts.second < sectorCount(ts.first))) {

				if (std::find(visited.begin(), visited.end(), ts) != visited.end()) {
					error("Directory chain loop detected");
					break;
				}
				visited.push_back(ts);

				auto sectorBuffer = sectorRead(ts);
				if (!sectorBuffer)
					break;

				// 8 entries per sector, 0x20 bytes per entry
				for (size_t i = 0; i <= 7; ++i) {
					auto file = filesystemEntryLoad(sectorBuffer, i * 0x20);
					if (!file) {
						pFile->mDirIndex.mTS = ts;
						pFile->mDirIndex.mOffset = i * 0x20;
						return pFile;
					}
				}

				// Reached end of directory?
				tTrackSector tsnext = { sectorBuffer->getByte(0), sectorBuffer->getByte(1) };
				if (tsnext.first == 0 || tsnext.second == 0) {
					tsnext.first = mDirectoryTrack;
					tsnext.second = ts.second + 3;
					if (!trackSectorValid(tsnext) || tsnext == ts) {
						return 0;
					}

					sectorBuffer->putByte(0, (uint8_t)tsnext.first);
					sectorBuffer->putByte(1, (uint8_t)tsnext.second);
					sectorWrite(ts, sectorBuffer);
				} else {
					if (!trackSectorValid(tsnext))
						break;
				}

				ts = tsnext;
			}
			// Directory full
			return 0;
		}

		/**
		 * Save changes to the filesystem
		 */
		bool cD64::filesystemSaveNative() { 
			for (auto node : mFsRoot->mNodes) {
				auto file = std::dynamic_pointer_cast<d64::sFile>(node);
				if (!file->isDirty())
					continue;

				// Totally new file?
				if (file->mDirIndex.mTS.first == 0) {
					if (!filesystemFindFreeIndex(file)) {
						// No space in directory
						return false;
					}
				}
				// If we have content
				if (file->mContent && file->mContent->size()) {
					auto sectorCount = file->mContent->size() / d64::gDataBytesPerSector;
					if (file->mContent->size() % d64::gDataBytesPerSector)
						++sectorCount;

					// Do we need more sectors?
					if (file->mSizeInSectors != sectorCount) {
							
						if (file->mChain.size() >= 1) {
							sectorsFree(file->mChain);
							file->mChain.clear();
						}

						file->mChain = sectorsUse(sectorCount);
					}

					if (file->mChain.size() == 0) {
						error("Not enough free space");
						continue;
					}

					file->mSizeInSectors = sectorCount;
					file->sizeInBytesSet(file->mSizeInSectors * (sectorSize() - 2));

					// Write out each sector in the chain
					for (size_t index = 0; index < file->mChain.size(); ++index ) {
						auto& ts = file->mChain[index];
						sAccessUnit tsnext;
							
						// Final sector?
						if(index < (file->mChain.size() - 1))
							tsnext = file->mChain[index + 1];
						else {
							tsnext.mTS = tTrackSector( 0, (uint8_t)(file->mContent->size() + 1) );
						}

						spBuffer buffer = std::make_shared<tBuffer>();
						buffer->pushByte((uint8_t)tsnext.track());
						buffer->pushByte((uint8_t)tsnext.sector());
						buffer->pushBuffer(file->mContent->takeBytes(d64::gDataBytesPerSector < file->mContent->size() ? d64::gDataBytesPerSector : file->mContent->size()));
						sectorWrite(ts.mTS, buffer);
					}
				} else {
					if (file->mChain.size()) {
						sectorsFree(file->mChain);
						file->mChain.clear();
					}
					file->mSizeInSectors = 0;
					file->sizeInBytesSet(0);
				}

				if (!filesystemEntrySave(file)) {
					return false;
				}
				file->dirty(false);
			}

			if (!filesystemBitmapSave()) {
				return false;
			}

			dirty(false);
			return true;
		}

		/**
		 *
		 */
		size_t cD64::filesystemTotalBytesFree() {
			return sectorsGetFree().size() * d64::gDataBytesPerSector;
		}

		/**
		 *
		 */
		size_t cD64::filesystemTotalBytesMax() {
			size_t total = 0;

			tTrackSector TS = { 1, 0 };

			for (; TS.first <= trackCount(); ++TS.first) {
				total += sectorCount(TS.first);
			}

			return total *= d64::gDataBytesPerSector;
		}

		/**
		 * Calculate number of sectors for this track
		 */
		tSector cD64::sectorCount(const tTrack pTrack) const {
			return sectorsPerTrack(pTrack);
		}

		/**
		 * Fixed sector size
		 */
		size_t cD64::sectorSize(const tTrack pTrack) const {
			return 256;
		}

		/**
		 * Is a sector free
		 *
		 * Ideally this would return in a 1541 drive read friendly manner
		 * Which is each sector used in a file is seperated by 10 sectors
		 */
		bool cD64::sectorIsFree(const tTrackSector pTS) const {
			if (pTS.first <= 0 || !d64::isTrackValidForImage(mImageType, pTS.first))
				return false;
			if (pTS.first - 1 >= mBam.size())
				return false;
			if (pTS.second >= sectorCount(pTS.first))
				return false;
			auto& Track = mBam[pTS.first - 1];
			return (Track.mSectors & ((uint64_t)1 << pTS.second)) != 0;
		}

		/**
		 * Set a sector used (True == used)
		 */
		bool cD64::sectorSet(const tTrackSector pTS, const bool pValue) {
			if (pTS.first <= 0 || !d64::isTrackValidForImage(mImageType, pTS.first))
				return false;
			if (pTS.first - 1 >= mBam.size())
				return false;
			if (pTS.second >= sectorCount(pTS.first))
				return false;
			auto& Track = mBam[pTS.first - 1];
			auto bit = ((uint64_t)1 << pTS.second);

			if (!pValue) {
				if(!(Track.mSectors & bit))
					++Track.mFreeSectors;
				Track.mSectors |= bit;
				return true;
			}

			if ((Track.mSectors & bit))
				--Track.mFreeSectors;
			Track.mSectors &= (~bit);
			return true;
		}

		/**
		 * Set sectors as free/used
		 */
		std::vector<sAccessUnit> cD64::sectorsUse(const tSector pTotal) {
			std::vector<sAccessUnit> results;
			tTrackSector TS = { 1, 0 };

			for (; TS.first <= trackCount(); ++TS.first) {
				for (TS.second = 0; TS.second < sectorCount(TS.first); ++TS.second) {
					if (sectorIsFree(TS)) {
						if (!sectorSet(TS, true)) {
							error("invalid sector used");
							return results;
						}

						// Do we have enough?
						results.push_back(TS);
						if (results.size() >= pTotal)
							return results;
					}
				}
			}

			error("Not enough free blocks");
			sectorsFree(results);
			return {};
		}

		/**
		 * Free sectors
		 */
		bool cD64::sectorsFree(const std::vector<sAccessUnit>& pSectors) {

			for (auto sector : pSectors) {
				if (!sectorSet(sector.mTS, false))
					return false;
			}
			return true;
		}

		/**
		 * Get free sectors on the disk. Track0 will return entire disk
		 */
		std::vector<sAccessUnit> cD64::sectorsGetFree(const tTrack pTrack) const {
			std::vector<sAccessUnit> results;
			tTrackSector TS = { pTrack, 0 };
			if (pTrack == 0) {
				for (++TS.first; TS.first <= trackCount(); ++TS.first) {
					for (TS.second = 0; TS.second < sectorCount(TS.first); ++TS.second) {
						if (sectorIsFree(TS)) {
							results.push_back(TS);
						}
					}
				}
			} else {
				for (TS.second = 0; TS.second < sectorCount(TS.first); ++TS.second) {
					if (sectorIsFree(TS)) {
						results.push_back(TS);
					}
				}
			}
			return results;
		}

		/**
		 * Load the T/S chain for a file
		 */
		bool cD64::filesystemChainLoad(spFile pFile) {
			auto file = std::dynamic_pointer_cast<d64::sFile>(pFile);
			if (!file)
				return false;
			if (file->mChain.empty())
				return false;

			tTrackSector ts = file->mChain[0].mTS;
			file->mChain.clear();
			std::vector<tTrackSector> visited;

			while (ts.first) {
				if (!trackSectorValid(ts)) {
					file->mChainBroken = true;
					return false;
				}
				if (std::find(visited.begin(), visited.end(), ts) != visited.end()) {
					file->mChainBroken = true;
					return false;
				}
				visited.push_back(ts);

				file->mChain.push_back(ts);

				auto sector = sectorRead(ts);
				if (!sector) {
					file->mChainBroken = true;
					return false;
				}

				// Next Track/Sector for this file
				ts = { sector->getByte(0), sector->getByte(1) };
			}
			return true;
		}

		/**
		 * Load the bitmap availability block
		 */
		bool cD64::filesystemBitmapLoad() {
			std::shared_ptr<firy::tBuffer> block;

			switch (mImageType) {
			case d64::eType_D81:
				block = sectorRead({ 40,0 });
				if (!block)
					return false;

				mDosVersion = block->getByte(0x02);
				mDiskID = block->getWordBE(0x10);
				mDosType = block->getWordBE(0x19);
				mFsName = block->getString(0x04, 16, 0xA0);
				break;

			default:
				block = sectorRead({ 18,0 });
				if (!block)
					return false;

				mDosVersion = block->getByte(0x02);
				mDiskID = block->getWordBE(0xA2);
				mDosType = block->getWordBE(0xA5);
				mFsName = block->getString(0x90, 16, 0xA0);
				break;
			}

			mBam.clear();
			tTrack track = 1;
			if (mImageType == d64::eType_D81) {
				if (!loadBamBlock({40, 1}, 1, 40, 6, 0x10, track))
					return false;
				track = 41;
				if (!loadBamBlock({40, 2}, 41, 80, 6, 0x10, track))
					return false;
			}
			else {
				if (!loadBamBlock({18, 0}, 1, 35, 4, 0x04, track))
					return false;
				if (mImageType == d64::eType_D71) {
					track = 36;
					if (!loadBamBlock({53, 0}, 36, 70, 4, 0x04, track))
						return false;
				}
			}

			switch (mDosType) {
			default:	// Unknown DOS
				break;

			case '2A':	// CBM DOS v2.6
			case '2P':	// PrologicDOS, ProSpeed
				if (mImageType != d64::eType_D81)
					return true;
				break;

			case '3D':
				if (mImageType == d64::eType_D81)
					return true;
				break;
			}

			return false;
		}

		/**
		 * Update the bitmap on the disk
		 */
		bool cD64::filesystemBitmapSave() {
			std::shared_ptr<firy::tBuffer> block;
			tTrack track = 1;

			switch (mImageType) {
			case d64::eType_D81:
				block = sectorRead({ 40,0 });
				if (!block)
					return false;

				block->putByte(0x02, mDosVersion);
				block->putWordBE(0x10, mDiskID);
				block->putWordBE(0x19, mDosType);

				{
					auto str = mFsName;
					str.resize(16, (char)0xA0);
					block->putString(0x04, str);
				}
				if (!sectorWrite({40,0}, block))
					return false;

				if (!saveBamBlock({40,1}, 1, 40, 6, 0x10, track))	// will be validated below
					return false;
				track = 41;
				if (!saveBamBlock({40,2}, 41, 80, 6, 0x10, track))
					return false;
				break;

			default:
				block = sectorRead({18,0});
				if (!block)
					return false;

				block->putByte(0x02, mDosVersion);
				block->putWordBE(0xA2, mDiskID);
				block->putByte(0xA4, 0xA0);
				block->putWordBE(0xA5, mDosType);

				{
					auto str = mFsName;
					str.resize(16, (char)0xA0);
					block->putString(0x90, str);
				}

				if (!sectorWrite({18,0}, block))
					return false;

				if (!saveBamBlock({18,0}, 1, 35, 4, 0x04, track))
					return false;
				if (mImageType == d64::eType_D71) {
					track = 36;
					if (!saveBamBlock({53,0}, 36, 70, 4, 0x04, track))
						return false;
				}
				break;
			}

			return true;
		}

		/**
		 * Load a file from the sector buffer
		 */
		d64::spFile cD64::filesystemEntryLoad(spBuffer pBuffer, const size_t pOffset) {
			d64::spFile file = filesystemNodeCreate<d64::sFile>(pBuffer->getString(pOffset + 0x05, 16, 0xA0));

			// Get the filetype
			file->mType = (d64::eFileType)(pBuffer->getByte(pOffset + 0x02) & 0x0F);
			file->mFlags = (pBuffer->getByte(pOffset + 0x02) & 0xF0);

			// Get the filename
			//file->nameSet(pBuffer->getString(pOffset + 0x05, 16, 0xA0));

			// Get the starting Track/Sector
			file->mChain.push_back(tTrackSector{ pBuffer->getByte(pOffset + 0x03), pBuffer->getByte(pOffset + 0x04) });

			if (file->mChain[0].track() == 0) {
				return 0;
			}
			// Total number of blocks
			file->mSizeInSectors = pBuffer->getWordLE(pOffset + 0x1E);
			file->sizeInBytesSet(file->mSizeInSectors * (sectorSize() - 2));

			helpers::sDateTime time(1982, 8, 1);	// Release date of C64
			file->timeWriteSet(time);

			file->dirty(false);
			return file;
		}

		/**
		 * Save a file to the sector buffer
		 */
		bool cD64::filesystemEntrySave(d64::spFile pFile) {
			auto sectorBuffer = sectorRead(pFile->mDirIndex.mTS);
			auto offset = pFile->mDirIndex.mOffset;
			tTrackSector startTs = pFile->mChain.empty() ? tTrackSector(0, 0) : pFile->mChain[0].mTS;

			// 
			sectorBuffer->putByte(offset + 0x02, pFile->mFlags | pFile->mType);

			auto str = str_to_upper(pFile->nameGet());
			str.resize(16, (char) 0xA0);
			sectorBuffer->putString(offset + 0x05, str);

			sectorBuffer->putByte(offset + 0x03, (uint8_t)startTs.first);
			sectorBuffer->putByte(offset + 0x04, (uint8_t)startTs.second);

			sectorBuffer->putWordLE(offset + 0x1E, (uint16_t)pFile->mSizeInSectors);
			if (!sectorWrite(pFile->mDirIndex.mTS, sectorBuffer))
				return false;

			pFile->dirty(false);
			return true;
		}
	}
}
