#include "firy.hpp"
#include "fat.hpp"

namespace firy {
	namespace images {

		using namespace fat;

		sFATFile::sFATFile(wpFilesystem pFilesystem) : sEntry(), filesystem::sFile(pFilesystem) {

		}

		sFATDir::sFATDir(wpFilesystem pFilesystem) : sEntry(), filesystem::sDirectory(pFilesystem) {

		}


		cFAT::cFAT(spSource pSource) : cDisk<interfaces::cBlocks>(pSource) {

			// Set a static blocksize
			mBlockSize = 512;	
			mBlockPartitionStart = 0;
			mBlockFAT = 0;
			mBlockRoot = 0;
			mType = eType::FAT12;
		}

		tBlock cFAT::fatSectorNext(tBlock pCluster) const {
			if (pCluster > mClusterMap.size()) {
				//std::cout << "Invalid Cluster\n";
				return 0;
			}

			auto next = mClusterMap[pCluster];
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
			if (mType != FAT32 && pStart == mClusterRoot && mBootBlock->mBiosParams.rootentries)
				return (mBootBlock->mBiosParams.rootentries * sizeof(fat::sFileEntry)) / blockSize();

			cl = pStart;
			while (pStart) {
				pStart = fatSectorNext(pStart);

				if(pStart)
					totalClusters++;
			}

			return totalClusters * mBootBlock->mBiosParams.secperclus;
		}

		tBlock cFAT::clusterToBlock(tBlock pCluster) const {
			return mBlockData + ((pCluster - 2) * mBootBlock->mBiosParams.secperclus);
		}

		/**
		 *
		 */
		std::string cFAT::filesystemNameGet() const {
			return std::string((const char*)mBootBlock->mBiosParam.label);
		}

		/**
		 *
		 */
		bool cFAT::filesystemPrepare() {
			mBootBlock = blockObjectGet<sBootRecordBlock>(0);
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
			mBootBlock = blockObjectGet<sBootRecordBlock>(pNumber);
			if (!mBootBlock)
				return false;

			mBlockPartitionStart = pNumber;
			mBlockFAT = mBlockPartitionStart + mBootBlock->mBiosParams.reserved;

			//FAT12 / FAT16
			if (mBootBlock->mBiosParams.secperfat && mBootBlock->mBiosParams.secperclus <= 128) {
				//if (!strncmp((const char*)mBootBlock->mBiosParam.system, "FAT", 3)) {

					// rootentries is set for fat12/16
					if (mBootBlock->mBiosParams.rootentries) {
						mBlockRoot = (mBlockFAT + (mBootBlock->mBiosParams.secperfat * 2));
						mBlockData = mBlockRoot + (((mBootBlock->mBiosParams.rootentries * sizeof(fat::sFileEntry)) + (512 - 1)) / 512);

						mClusterRoot = mBlockRoot / mBootBlock->mBiosParams.secperclus;
					}
				//}
			} else {
				if (mBootBlock->mBiosParam32.fatsize) {
					mBlockData = mBootBlock->mBiosParams.hidden + ((mBootBlock->mBiosParam32.fatsize * mBootBlock->mBiosParams.numfats) + mBootBlock->mBiosParams.reserved);
					mBlockRoot = mBlockData + (mBlockRoot * mBootBlock->mBiosParams.secperclus);
				
					mClusterRoot = blockToCluster(mBlockRoot);
				}
			}

			mClustersTotal = (blockCount() - mBlockData) / mBootBlock->mBiosParams.secperclus;
			if (mClustersTotal < 4085)
				mType = FAT12;
			else if (mClustersTotal < 65525)
				mType = FAT16;
			else
				mType = FAT32;

			clusterMapLoad();

			auto Root = std::make_shared<sFATDir>(weak_from_this());
			Root->mFirstCluster = mClusterRoot;
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
				auto blockFat = sourceBufferCopy(mBlockFAT * blockSize(), 4608 * 3);
				if (!blockFat)
					return false;

				for (size_t i = 0, j = 0; i < 4608; i += 3) {
					mClusterMap.push_back((blockFat->at(i) + (blockFat->at(i + 1) << 8)) & 0x0FFF);
					mClusterMap.push_back((blockFat->at(i + 1) + (blockFat->at(i + 2) << 8)) >> 4);
				}
			}
			else if (mType == FAT16) {
				size_t size = (mBootBlock->mBiosParams.secperfat * mBootBlock->mBiosParams.numfats) * blockSize();
				auto blockFat = sourceBufferCopy(mBlockFAT * blockSize(), size);
				if (!blockFat)
					return false;

				for (size_t i = 0; i < size; i += 2) {
					mClusterMap.push_back((blockFat->at(i) + (blockFat->at(i + 1) << 8)) & 0xFFFF);
				}
			} else {
				size_t size = (mBlockData - mBlockFAT) * blockSize();
				auto blockFat = sourceBufferCopy(mBlockFAT * blockSize(), size);
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
		spBuffer cFAT::clusterChainReadRoot(size_t pStartBlock) {
			size_t totalbytes = directorySectors(mClusterRoot) * blockSize();
			auto buffer = std::make_shared<tBuffer>();
			buffer->resize(totalbytes);
			auto dataptr = (uint8_t*)buffer->data();
			auto sector = pStartBlock;

			while (totalbytes) {
				auto size = min(totalbytes, mBootBlock->mBiosParams.secperclus * blockSize());
				auto block = sourceBufferCopy(sector * blockSize(), mBootBlock->mBiosParams.secperclus * blockSize());

				memcpy(dataptr, block->data(), size);
				dataptr += size;
				totalbytes -= size;

				pStartBlock = fatSectorNext(pStartBlock);
				if (!pStartBlock && totalbytes) {
					// TODO: Error
					return {};
				}
				sector = pStartBlock;
			}
			return buffer;
		}

		/**
		 * Read and follow a chain of clusters
		 */
		spBuffer cFAT::clusterChainRead(size_t pCluster) {
			size_t totalbytes = directorySectors(pCluster) * blockSize();
			auto buffer = std::make_shared<tBuffer>();
			buffer->resize(totalbytes);
			auto dataptr = (uint8_t*) buffer->data();

			while (totalbytes) {
				auto sector = clusterToBlock(pCluster);

				auto size = min(totalbytes, mBootBlock->mBiosParams.secperclus * blockSize());
				auto block = sourceBufferCopy(sector * blockSize(), mBootBlock->mBiosParams.secperclus * blockSize());

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

		/**
		 * Load in a cluster of a directory listing
		 */
		bool cFAT::entrysLoad(spFATDir pDir) {
			size_t cluster = pDir->mFirstCluster;

			spBuffer block;
			
			if(pDir == mFsRoot && mType != FAT32)
				block = clusterChainReadRoot(pDir->mBlock);
			else
				block = clusterChainRead(pDir->mFirstCluster);

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

		/**
		 * Load an individual entry in a directory listing
		 */
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
			entry->mBlock = clusterToBlock(StartCluster);
			result->mName.append((const char*)pEntry->Name, 8);
			result->mName = rtrim(result->mName, 0x20);	// Trim spaces

			if (!pEntry->Attributes.directory) {
				result->mName.append(".");
				result->mName.append((const char*)pEntry->Extension, 3);
				result->mName = rtrim(result->mName, 0x20);	// Trim spaces
			}
			return result;
		}

		/**
		 * Read a file from the filesystem
		 */
		spBuffer cFAT::filesystemRead(spNode pFile) {
			spFATFile File = std::dynamic_pointer_cast<sFATFile>(pFile);
			if (!File)
				return {};

			auto buffer = clusterChainRead(File->mFirstCluster);
			buffer->resize(File->mSizeInBytes);
			return buffer;
		}

		/**
		 *
		 */
		tBlock cFAT::blockCount() const {
			return (mBootBlock->mBiosParams.sectors_s ? mBootBlock->mBiosParams.sectors_s : mBootBlock->mBiosParams.sectors_l);
		}

		/**
		 *
		 */
		size_t cFAT::blockSize(const tBlock pBlock) const {

			if (!mBootBlock)
				return cBlocks::blockSize(pBlock);

			return mBootBlock->mBiosParams.bytepersec;
		}

		/**
		 *
		 */
		tBlock cFAT::blockToCluster(const tBlock pBlock) const {
			return ((pBlock - mBlockData) / mBootBlock->mBiosParams.secperclus) + 2;
		}

		bool cFAT::blockIsFree(const tBlock pBlock) const {
			return false;
		}

		/**
		 * Return free clusters
		 */
		std::vector<tBlock> cFAT::blocksFree() const {
			std::vector<tBlock> free;
			tBlock clusternumber = 0;

			for (auto& cluster : mClusterMap) {
				if (!cluster) {
					auto block = clusterToBlock(clusternumber);
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
