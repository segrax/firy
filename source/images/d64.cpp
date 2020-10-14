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

			// TODO: Determine tracks based on filesize
			mTrackCount = 35;
		}

		/**
		 * Get the name of the disk
		 */
		std::string cD64::filesystemNameGet() const {
			return mLabel;
		}

		/**
		 * 
		 */
		void cD64::filesystemNameSet(const std::string& pName) {
			mLabel = pName;
		}

		/**
		 * Create an empty filesystem
		 */
		bool cD64::filesystemCreate() {
			mFsRoot = std::make_shared<firy::filesystem::sDirectory>(weak_from_this(), "/");

			sourceChunkPrepare(174848);
			mDosVersion = 0x21;
			mDiskID = 'FI';
			mDosType = '2A';
			mLabel = "";

			tTrackSector ts = { 1, 4 };
			mBam.clear();
			for (; ts.first < 35; ++ts.first) {
				d64::sTrackBam bam;
				mBam.push_back(bam);
			}
			for (ts.first = 1; ts.first < 35; ++ts.first) {
				for (ts.second = 0; ts.second < sectorCount(ts.first); ++ts.second) {
					sectorSet(ts, false);
				}
			}

			filesystemBitmapSave();
			return filesystemBitmapLoad();
		}

		/**
		 * Load the D64 directory
		 */
		bool cD64::filesystemLoad() {
			mFsRoot = std::make_shared<firy::filesystem::sDirectory>(weak_from_this(), "/");

			// Loop until we reach the end of the directory
			tTrackSector ts(18, 1);
			while ((ts.first > 0 && ts.first <= trackCount()) &&
				(ts.second <= sectorCount(ts.first))) {

				auto sectorBuffer = sectorRead(ts);
				if (!sectorBuffer)
					break;

				// 8 entries per sector, 0x20 bytes per entry
				for (size_t i = 0; i <= 7; ++i) {
					d64::spFile file = filesystemEntryLoad(sectorBuffer, i * 0x20);
					if (file) {
						file->mDirIndex.mTS = ts;
						file->mDirIndex.mOffset = i * 20;
						mFsRoot->mNodes.push_back(file);
					}
				}

				// Get the next Track/Sector in the chain
				tTrackSector nextTs = { sectorBuffer->getByte(0), sectorBuffer->getByte(1)};
				if (nextTs == ts)
					break;
				ts = std::move(nextTs);
			}
			return filesystemBitmapLoad();
		}

		/**
		 * Load a file off the D64
		 */
		spBuffer cD64::filesystemRead(spNode pNode) {
			uint16_t bytesCopied = 0, copySize = d64::gDataBytesPerSector;
			d64::spFile File = std::dynamic_pointer_cast<d64::sFile>(pNode);
			if (!File)
				return 0;

			if (!filesystemChainLoad(File))
				return 0;

			// Prepare a buffer to hold the file
			spBuffer buffer = std::make_shared<tBuffer>();
			buffer->resize(File->mSizeInBytes);
			uint8_t* destBuffer = buffer->data();

			for (auto& ts : File->mChain) {
				bool noCopy = false;

				uint8_t* sectorptr = sourceChunkPtr(sectorOffset(ts));
				if (!sectorptr) {
					File->mChainBroken = true;
					return buffer;
				}

				// T/S is broken, or the directory entry is wrong about size
				if (bytesCopied >= buffer->size())
					noCopy = true;

				// Last Sector of file? 
				if (!sectorptr[0]) {
					// Bytes used by the final sector stored in the T/S chain sector value
					copySize = (uint16_t)(sectorptr[1] - 1);
					// Adjust bufer size to match the final file size
					buffer->resize(buffer->size() - (d64::gDataBytesPerSector - copySize));
				}

				// Copy sector data, excluding the T/S Chain data
				if (!noCopy)
					memcpy(destBuffer, sectorptr + 2, copySize);

				// Move the dest buffer forward
				destBuffer += copySize;
				bytesCopied += copySize;
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
			file->mSizeInSectors = 0;
			sectorsFree(file->mChain);
			file->mChain.clear();
			return filesystemSave();
		}

		/**
		 * Find a free space in the directory index
		 */
		d64::spFile cD64::filesystemFindFreeIndex(d64::spFile pFile) {
			tTrackSector ts(18, 1);
			while ((ts.first > 0 && ts.first <= trackCount()) &&
				(ts.second <= sectorCount(ts.first))) {

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
					tsnext.first = 18;
					tsnext.second = ts.second + 3;

					sectorBuffer->putByte(0, (uint8_t)tsnext.first);
					sectorBuffer->putByte(1, (uint8_t)tsnext.second);
					sectorWrite(ts, sectorBuffer);
				}

				ts = tsnext;
			}
			// Directory full
			return 0;
		}

		/**
		 * Save changes to the filesystem
		 */
		bool cD64::filesystemSave() { 
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
				if (file->mContent->size()) {
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
						gDebug->error("No disk space");
						continue;
					}

					file->mSizeInSectors = sectorCount;
					file->mSizeInBytes = file->mSizeInSectors * (sectorSize() - 2);

					// Write out each sector in the chain
					for (size_t index = 0; index < file->mChain.size(); ++index ) {
						auto& ts = file->mChain[index];
						tTrackSector tsnext;
							
						// Final sector?
						if(index < (file->mChain.size() - 1))
							tsnext = file->mChain[index + 1];
						else {
							tsnext = { 0, file->mContent->size() + 1 };
						}

						spBuffer buffer = std::make_shared<tBuffer>();
						buffer->pushByte((uint8_t)tsnext.first);
						buffer->pushByte((uint8_t)tsnext.second);
						buffer->pushBuffer(file->mContent->takeBytes(d64::gDataBytesPerSector < file->mContent->size() ? d64::gDataBytesPerSector : file->mContent->size() ));
						sectorWrite(ts, buffer);
					}
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
		 * Calculate number of sectors for this track
		 */
		tSector cD64::sectorCount(const tTrack pTrack) const {
			return ((21 - (pTrack > 17) * 2) - (pTrack > 24) - (pTrack > 30));
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
			if (pTS.first - 1 >= mBam.size())
				return false;
			auto& Track = mBam[pTS.first - 1];
			return (Track.mSectors & (1 << pTS.second));
		}

		/**
		 * Set a sector used (True == used)
		 */
		bool cD64::sectorSet(const tTrackSector pTS, const bool pValue) {
			if (pTS.first - 1 >= mBam.size())
				return false;
			auto& Track = mBam[pTS.first - 1];
			auto bit = (1 << pTS.second);

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
		std::vector<tTrackSector> cD64::sectorsUse(const tSector pTotal) {
			std::vector<tTrackSector> results;
			tTrackSector TS = { 1, 0 };

			for (; TS.first < trackCount(); ++TS.first) {
				for (TS.second = 0; TS.second < sectorCount(TS.first); ++TS.second) {
					if (sectorIsFree(TS)) {
						if (!sectorSet(TS, true)) {
							gDebug->error("invalid sector");
							return results;
						}

						// Do we have enough?
						results.push_back(TS);
						if (results.size() >= pTotal)
							return results;
					}
				}
			}

			gDebug->error("Not enough free blocks");
			sectorsFree(results);
			return {};
		}

		/**
		 * Free sectors
		 */
		bool cD64::sectorsFree(const std::vector<tTrackSector>& pSectors) {

			for (auto sector : pSectors) {
				if (!sectorSet(sector, false))
					return false;
			}
			return true;
		}

		/**
		 * Get free sectors on the disk. Track0 will return entire disk
		 */
		std::vector<tTrackSector> cD64::sectorsGetFree(const tTrack pTrack) const {
			std::vector<tTrackSector> results;
			tTrackSector TS = { pTrack, 0 };
			if (pTrack == 0) {
				for (++TS.first; TS.first < trackCount(); ++TS.first) {
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

			tTrackSector ts = file->mChain[0];
			file->mChain.clear();

			while (ts.first) {
				file->mChain.push_back(ts);

				// Track/Sector already in use?
				//if (mBamRealTracks[currentTrack][currentSector]) {
					// Add to the crosslinked list
				//	mCrossLinked.push_back(sD64Chain(currentTrack, currentSector, pFile));
				//} else
				//	mBamRealTracks[currentTrack][currentSector] = pFile;

				uint8_t* sectorptr = sourceChunkPtr(sectorOffset(ts));
				if (!sectorptr) {
					file->mChainBroken = true;
					return false;
				}

				// Next Track/Sector for this file
				ts = { sectorptr[0], sectorptr[1] };
			}
			return true;
		}

		/**
		 * Load the bitmap availability block
		 */
		bool cD64::filesystemBitmapLoad() {
			auto block = sectorRead({ 18,0 });
			if (!block)
				return false;

			mDosVersion = block->getByte(0x02);
			mDiskID = block->getWordBE(0xA2);
			mDosType = block->getWordBE(0xA5);

			mLabel = block->getString(0x90, 16, 0xA0); 

			// Load the BAM (We are abusing the sector field to hold the byte offset
			tTrackSector ts = { 1, 4 };
			mBam.clear();
			for (; ts.first < 35; ++ts.first) {
				d64::sTrackBam bam;

				bam.mFreeSectors = block->getByte(ts.second++);
				bam.mSectors = block->getByte(ts.second++);
				bam.mSectors |= block->getByte(ts.second++) << 8;
				bam.mSectors |= block->getByte(ts.second++) << 16;

				mBam.push_back(bam);
			}

			// Do we recognise the disk DOS type
			switch (mDosType) {
				default:	// Unknown DOS
					break;

				case '2A':	// CBM DOS v2.6
				case '2P':	// PrologicDOS, ProSpeed 
					return true;
			}

			return false;
		}

		/**
		 * Update the bitmap on the disk
		 */
		bool cD64::filesystemBitmapSave() {
			auto block = sectorRead({ 18,0 });
			if (!block)
				return false;

			block->putByte(0x02, mDosVersion);
			block->putWordBE(0xA2, mDiskID);
			block->putByte(0xA4, 0xA0);
			block->putWordBE(0xA5, mDosType);

			auto str = mLabel;
			str.resize(16, (char)0xA0);

			block->putString(0x90, str);
			tTrackSector ts = { 1, 4 };

			for (auto bam : mBam) {
				block->putByte(ts.second++, bam.mFreeSectors );
				block->putByte(ts.second++, bam.mSectors);
				block->putByte(ts.second++, bam.mSectors >> 8);
				block->putByte(ts.second++, bam.mSectors >> 16);
			}

			return sectorWrite({ 18,0 }, block);
		}

		/**
		 * Load a file from the sector buffer
		 */
		d64::spFile cD64::filesystemEntryLoad(spBuffer pBuffer, const size_t pOffset) {
			d64::spFile file = std::make_shared<d64::sFile>(weak_from_this());

			// Get the filetype
			file->mType = (d64::eFileType)(pBuffer->getByte(pOffset + 0x02) & 0x0F);
			file->mFlags = (pBuffer->getByte(pOffset + 0x02) & 0xF0);

			// Get the filename
			file->nameSet(pBuffer->getString(pOffset + 0x05, 16, 0xA0));

			// Get the starting Track/Sector
			file->mChain.emplace_back( pBuffer->getByte(pOffset + 0x03), pBuffer->getByte(pOffset + 0x04));

			if (file->mChain[0].first == 0) {
				return 0;
			}
			// Total number of blocks
			file->mSizeInSectors = pBuffer->getWordLE(pOffset + 0x1E);
			file->mSizeInBytes = file->mSizeInSectors * (sectorSize() - 2);
			file->dirty(false);
			return file;
		}

		/**
		 * Save a file to the sector buffer
		 */
		bool cD64::filesystemEntrySave(d64::spFile pFile) {
			auto sectorBuffer = sectorRead(pFile->mDirIndex.mTS);
			auto offset = pFile->mDirIndex.mOffset;

			// 
			sectorBuffer->putByte(offset + 0x02, pFile->mFlags | pFile->mType);

			auto str = str_to_upper(pFile->nameGet());
			str.resize(16, (char) 0xA0);
			sectorBuffer->putString(offset + 0x05, str);

			sectorBuffer->putByte(offset + 0x03, (uint8_t)pFile->mChain[0].first);
			sectorBuffer->putByte(offset + 0x04, (uint8_t)pFile->mChain[0].second);

			sectorBuffer->putWordLE(offset + 0x1E, (uint16_t)pFile->mSizeInSectors);
			if (!sectorWrite(pFile->mDirIndex.mTS, sectorBuffer))
				return false;

			pFile->dirty(false);
			return true;
		}
	}
}
