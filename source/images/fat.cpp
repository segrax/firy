#include "firy.hpp"
#include "fat.hpp"

namespace firy {
	namespace images {

		using namespace fat;

		sFATFile::sFATFile(wpFilesystem pFilesystem) : sEntry(), filesystem::sFile(pFilesystem) {

		}

		sFATDir::sFATDir(wpFilesystem pFilesystem) : sEntry(), filesystem::sDirectory(pFilesystem) {

		}

		tBlock cFAT::fatSectorNext(tBlock pCurrent) const {
			if (pCurrent > mClusterMap.size()) {
				//std::cout << "Invalid Cluster\n";
				return 0;
			}

			auto next = mClusterMap[pCurrent];
			if ((next == 0) || 
				(mType == FAT12 && next >= 0x0FF0 && next <= 0x0FFF) || 
				(mType == FAT16 && next >= 0xFFF0 && next <= 0xFFFF) ||
				(mType == FAT32 && next >= 0x0FFFFFF0 && next <= 0x0FFFFFFF))
				return 0;
			
			return next & 0x0FFFFFFF;
		}

		tBlock cFAT::directorySectors(tBlock pStart) const {
			tBlock totalClusters = 1, cl;

			// The root directory (start=0) has a fixed size.
			if (!pStart && mBootBlock->mBiosParams.rootentries)
				return (mBootBlock->mBiosParams.rootentries * sizeof(fat::sFileEntry)) / blockSize();

			cl = pStart;
			while (pStart) {
				pStart = fatSectorNext(pStart);

				if(pStart)
					totalClusters++;
			}

			return totalClusters * mBootBlock->mBiosParams.secperclus;
		}

		tBlock cFAT::getFirstDataSector() const {
			return (mBootBlock->mBiosParam32.fatsize * mBootBlock->mBiosParams.numfats) + mBootBlock->mBiosParams.reserved;
		}

		tBlock cFAT::clusterToBlock(tBlock pCluster, bool pIsRoot) const {
			if (mType == FAT32) {
				auto data_start = getFirstDataSector() + mBootBlock->mBiosParams.hidden;
				return data_start + ((pCluster - 2) * mBootBlock->mBiosParams.secperclus);
			}

			if (pIsRoot) {
				return ((pCluster)* mBootBlock->mBiosParams.secperclus);
			}

			auto dir_start = mBlockPartitionStart + mBootBlock->mBiosParams.numfats * mBootBlock->mBiosParams.secperfat + (mType == FAT32 ? 1 : 0);
			auto data_start = dir_start + (mBootBlock->mBiosParams.rootentries * sizeof(sFileEntry) / mBootBlock->mBiosParams.bytepersec);
			return data_start + ((pCluster - 1) * mBootBlock->mBiosParams.secperclus);
		}

		/**
		 * Maximum number of bytes which can be stored in a block
		 */
		const size_t gBytesPerBlock = 512;

		cFAT::cFAT() : cDisk<interfaces::cBlocks>() {
			mBlockPartitionStart = 0;
			mBlockSize = gBytesPerBlock;
			mBlockFAT = 0;
			mBlockRoot = 0;
			mType = eType::FAT12;
		}

		/**
		 *
		 */
		std::string cFAT::filesystemNameGet() const {
			return "";
		}

		/**
		 *
		 */
		bool cFAT::filesystemPrepare() {
			mBootBlock = blockLoad<sBootRecordBlock>(0);
			if (!mBootBlock)
				return false;

			// Removable
			if (mBootBlock->mBiosParams.mediatype & 0xF0 && mBootBlock->mBiosParams.bytepersec < 0x1000) {
				return partitionOpen(0);
			}

			// Check the partition table
			for (int part = 0; part < 4; ++part) {

				if (mBootBlock->mbr.parts[part].boot == 0) {
					continue;
				}

				if (mBootBlock->mbr.parts[part].boot == 0x80) {
					return partitionOpen(mBootBlock->mbr.parts[part].firstSector);
				}
			}

			return partitionOpen(0);
		}

		/**
		 *
		 */
		bool cFAT::partitionOpen(int pNumber) {
			mBootBlock = blockLoad<sBootRecordBlock>(pNumber);
			if (!mBootBlock)
				return false;

			mBlockPartitionStart = pNumber;
			mBlockFAT = mBootBlock->mBiosParams.reserved + mBlockPartitionStart;

			tBlock clusterCount;

			//FAT12 / FAT16
			if (mBootBlock->mBiosParams.secperfat) {
				if (strcmp((const char*)mBootBlock->mBiosParam.system, "FAT")) {

					// rootentries is set for fat12/16
					if (mBootBlock->mBiosParams.rootentries) {
						mBlockRoot = (mBlockFAT + (mBootBlock->mBiosParams.secperfat * 2));
						mBlockData = mBlockRoot + (((mBootBlock->mBiosParams.rootentries * sizeof(fat::sFileEntry)) + (512 - 1)) / 512);

						mBlockRoot = mBlockRoot / mBootBlock->mBiosParams.secperclus;
						tBlock totalUsed = mBootBlock->mBiosParams.reserved + mBootBlock->mBiosParams.numfats * mBootBlock->mBiosParams.secperfat + mBootBlock->mBiosParams.rootentries * sizeof(sFileEntry) / mBootBlock->mBiosParams.bytepersec;
						clusterCount = (blockCount() - totalUsed) / mBootBlock->mBiosParams.secperclus;
						if (clusterCount < 4085)
							mType = FAT12;
						else
							mType = FAT16;
					}

				}
			}
			else {
				if (mBootBlock->mBiosParam32.fatsize) {
					mType = FAT32;

					mBlockData = mBlockPartitionStart + (mBootBlock->mBiosParams.reserved + (mBootBlock->mBiosParams.numfats * mBootBlock->mBiosParam32.fatsize));
					mBlockRoot = blockToCluster(mBlockData + ((mBlockRoot) * mBootBlock->mBiosParams.secperclus));

					tBlock totalUsed = mBootBlock->mBiosParams.reserved + mBootBlock->mBiosParams.numfats * mBootBlock->mBiosParams.secperfat + mBootBlock->mBiosParams.rootentries * sizeof(sFileEntry) / mBootBlock->mBiosParams.bytepersec;
					clusterCount = (blockCount() - totalUsed) / mBootBlock->mBiosParams.secperclus;
				}
			}

			clusterMapLoad();

			auto Root = std::make_shared<sFATDir>(weak_from_this());
			Root->mFirstCluster = mBlockRoot;
			Root->mBlock = mBlockRoot;
			Root->mSizeInBytes = mBootBlock->mBiosParams.rootentries * sizeof(fat::sFileEntry);

			if (!Root->mSizeInBytes) {
				Root->mSizeInBytes = directorySectors(blockToCluster(Root->mBlock)) * blockSize();
			}
			mFsRoot = Root;
			return entrysLoad(Root);
		}

		/**
		 *
		 */
		bool cFAT::clusterMapLoad() {
			mClusterMap.clear();

			if (mType == FAT12) {
				auto blockFat = imageBufferCopy(mBlockFAT * blockSize(), 4608 * 3);
				if (!blockFat)
					return false;

				for (size_t i = 0, j = 0; i < 4608; i += 3) {
					mClusterMap.push_back((blockFat->at(i) + (blockFat->at(i + 1) << 8)) & 0x0FFF);
					mClusterMap.push_back((blockFat->at(i + 1) + (blockFat->at(i + 2) << 8)) >> 4);
				}
			}
			else if (mType == FAT16) {
				size_t size = (mBootBlock->mBiosParams.secperfat * mBootBlock->mBiosParams.numfats) * blockSize();

				auto blockFat = imageBufferCopy(mBlockFAT * blockSize(), size);
				if (!blockFat)
					return false;

				for (size_t i = 0; i < size; i += 2) {
					mClusterMap.push_back((blockFat->at(i) + (blockFat->at(i + 1) << 8)) & 0xFFFF);
				}
			} else {
				size_t size = (mBlockData - mBlockFAT) * blockSize();

				auto blockFat = imageBufferCopy(mBlockFAT * blockSize(), size);
				if (!blockFat)
					return false;

				for (size_t i = 0; i < size; i += 4) {
					uint32_t val = ((blockFat->at(i) |
						(blockFat->at(i + 1) << 8)) |
						(blockFat->at(i + 2) << 16) |
						(blockFat->at(i + 3) << 24));


					mClusterMap.push_back(val & 0x0fffffff);
				}
			}

			return true;
		}

		std::shared_ptr<tBuffer> cFAT::clusterChainRead(size_t pCluster, bool pIsRoot) {
			size_t totalbytes = directorySectors(pCluster) * blockSize();
			auto buffer = std::make_shared<tBuffer>();
			buffer->resize(totalbytes);
			auto dataptr = (uint8_t*) buffer->data();

			while (totalbytes) {
				auto sector = clusterToBlock(pCluster, pIsRoot);

				auto size = min(totalbytes, mBootBlock->mBiosParams.secperclus * blockSize());
				auto block = imageBufferCopy(sector * blockSize(), mBootBlock->mBiosParams.secperclus * blockSize());

				memcpy(dataptr, block->data(), size);
				dataptr += size;
				totalbytes -= size;

				pCluster = fatSectorNext(pCluster);
				if (!pCluster && totalbytes) {
					// TODO: Error
					return {};
				}
			}
			return buffer;
		}

		bool cFAT::entrysLoad(spFATDir pDir) {
			size_t cluster = pDir->mFirstCluster;

			auto block = clusterChainRead(pDir->mFirstCluster, pDir == mFsRoot);

			sFileEntry* Entry = (sFileEntry*) block->data();
			sFileEntry* LastEntry = Entry + (block->size() / sizeof(fat::sFileEntry));

			for (; Entry != LastEntry; ++Entry) {
				if (!Entry->Name[0]) {
					break;
				}

				if (Entry->Name[0] == 0x20 || Entry->Name[0] > 0x80)
					continue;

				// Long File Name
				if (Entry->Attribute == 0x0F)
					continue;

				auto entry = entryLoad(Entry, 0);
				if (entry) {
					if (typeid(*entry) == typeid(sFATDir)) {
						if (entry->mName == "." || entry->mName == "..")
							continue;
						entrysLoad(std::dynamic_pointer_cast<sFATDir>(entry));
					}
					pDir->mNodes.push_back(entry);
				}
			}


			return true;
		}

		spNode cFAT::entryLoad(const fat::sFileEntry* pEntry, const tBlock pBlock) {
			spNode result;
			sEntry* entry = 0;

			auto StartCluster = (uint32_t)pEntry->StartCluster;

			if (mType == FAT32) {
				StartCluster |= ((uint32_t)pEntry->StartClusterHi) << 16;
			}
			if (pEntry->Attributes.directory) {
				spFATDir Dir = std::make_shared<sFATDir>(weak_from_this());
				result = Dir;
				entry = Dir.operator->();


				Dir->mSizeInBytes = directorySectors(StartCluster) * blockSize();
			} else {
				spFATFile File = std::make_shared<sFATFile>(weak_from_this());
				result = File;
				entry = File.operator->();

				File->mSizeInBytes = pEntry->FileLength;
			}

			entry->mFirstCluster = StartCluster;
			entry->mBlock = clusterToBlock(StartCluster, false);
			result->mName.append((const char*)pEntry->Name, 8);
			result->mName = rtrim(result->mName, 0x20);	// Trim spaces

			if (!pEntry->Attributes.directory) {
				result->mName.append(".");
				result->mName.append((const char*)pEntry->Extension, 3);
				result->mName = rtrim(result->mName, 0x20);	// Trim spaces
			}
			if (result->mName == "SYSTEM") {
				std::cout << "a";
			}
			return result;
		}

		spBuffer cFAT::filesystemRead(spNode pFile) {
			spFATFile File = std::dynamic_pointer_cast<sFATFile>(pFile);
			if (!File)
				return {};

			auto buffer = clusterChainRead(File->mFirstCluster, false);
			buffer->resize(File->mSizeInBytes);
			return buffer;
		}

		tBlock cFAT::blockCount() const {
			return (mBootBlock->mBiosParams.sectors_s ? mBootBlock->mBiosParams.sectors_s : mBootBlock->mBiosParams.sectors_l);
		}

		size_t cFAT::blockSize(const tBlock pBlock) const {
			if (mType == FAT32)
				return 512;

			if (!mBootBlock)
				return cBlocks::blockSize(pBlock);

			return mBootBlock->mBiosParams.bytepersec;
		}

		tBlock cFAT::blockToCluster(const tBlock pBlock) const {
			return ((pBlock - mBlockData) / mBootBlock->mBiosParams.secperclus) + 2;
		}

		bool cFAT::blockIsFree(const tBlock pBlock) const {
			return false;
		}

		std::vector<tBlock> cFAT::blocksFree() const {
			std::vector<tBlock> free;
			tBlock clusternumber = 0;

			for (auto& cluster : mClusterMap) {
				if (!cluster) {
					auto block = clusterToBlock(clusternumber, false);
					for(auto current = block; current < block + mBootBlock->mBiosParams.secperclus; ++current)
						free.push_back( current );
				}
				++clusternumber;
			}
			return free;
		}

		bool cFAT::filesystemChainLoad(spFile pFile) {
			return false;
		}

		spFATFile cFAT::filesystemEntryProcess(const uint8_t* pBuffer) {
			return {};
		}

		bool cFAT::filesystemBitmapLoad() {
			return false;
		}


	}
}
