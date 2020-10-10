#include "firy.hpp"
#include "fat.hpp"

namespace firy {
	namespace images {

		fat::sFile::sFile(wpFilesystem pFilesystem) : sEntry(), filesystem::sFile(pFilesystem) {

		}

		fat::sDir::sDir(wpFilesystem pFilesystem) : sEntry(), filesystem::sDirectory(pFilesystem) {

		}


		cFAT::cFAT(spSource pSource) : cImageAccess<access::cBlocks>(), access::cInterface(pSource) {

			// Set a static blocksize
			mBlockSize = 512;	
			mBlockPartitionStart = 0;
			mBlockFAT = 0;
			mBlockRoot = 0;
			mType = fat::eType::FAT12;
		}

		tBlock cFAT::fatSectorNext(tBlock pCluster) const {
			if (pCluster > mClusterMap.size()) {
				//std::cout << "Invalid Cluster\n";
				return 0;
			}

			auto next = mClusterMap[pCluster];
			if ((next == 0) || 
				(mType == fat::FAT12 && next >= 0x0FF0 && next <= 0x0FFF) ||
				(mType == fat::FAT16 && next >= 0xFFF0 && next <= 0xFFFF) ||
				(mType == fat::FAT32 && next >= 0x0FFFFFF0 && next <= 0x0FFFFFFF))
				return 0;
			
			return next & 0x0FFFFFFF;
		}

		tBlock cFAT::directorySectors(tBlock pStart) const {
			tBlock totalClusters = 1, cl;

			// The root directory (start=0) has a fixed size.
			if (mType != fat::FAT32 && pStart == mClusterRoot && mBootBlock->mBiosParams.mRootEntryCount)
				return (mBootBlock->mBiosParams.mRootEntryCount * sizeof(fat::sFileEntry)) / blockSize();

			cl = pStart;
			while (pStart) {
				pStart = fatSectorNext(pStart);

				if(pStart)
					totalClusters++;
			}

			return totalClusters * mBootBlock->mBiosParams.mSectorsPerCluster;
		}

		/**
		 * Convert a cluster number to a block
		 */
		tBlock cFAT::clusterToBlock(tBlock pCluster) const {
			return mBlockData + ((pCluster - 2) * mBootBlock->mBiosParams.mSectorsPerCluster);
		}

		/**
		 * 
		 */
		std::string cFAT::filesystemNameGet() const {
			return std::string(mBootBlock->mBiosParam.mLabel, strlen(mBootBlock->mBiosParam.mLabel));
		}

		/**
		 *
		 */
		bool cFAT::filesystemPrepare() {
			auto bootBlock = blockObjectGet<fat::sBootRecordBlock>(0);
			if (!bootBlock)
				return false;

			// Check the partition table
			for (int part = 0; part < 4; ++part) {
				if (bootBlock->mMasterBootRecord.mPartitions[part].mActive == 0) {
					continue;
				}
				// Active?
				if (bootBlock->mMasterBootRecord.mPartitions[part].mActive == 0x80) {
					return partitionOpen(bootBlock->mMasterBootRecord.mPartitions[part].mStartLBA);
				}
			}

			return partitionOpen(0);
		}

		/**
		 *
		 */
		bool cFAT::partitionOpen(int pNumber) {
			mBootBlock = blockObjectGet<fat::sBootRecordBlock>(pNumber);
			if (!mBootBlock)
				return false;

			mBlockPartitionStart = pNumber;
			mBlockFAT = mBlockPartitionStart + mBootBlock->mBiosParams.mSectorsReserved;

			//FAT12 / FAT16
			if (mBootBlock->mBiosParams.mSectorsPerFAT && mBootBlock->mBiosParams.mSectorsPerCluster <= 128) {
				//if (!strncmp((const char*)mBootBlock->mBiosParam.system, "FAT", 3)) {

					if (mBootBlock->mBiosParams.mRootEntryCount) {
						mBlockRoot = (mBlockFAT + (mBootBlock->mBiosParams.mSectorsPerFAT * 2));
						mBlockData = mBlockRoot + (((mBootBlock->mBiosParams.mRootEntryCount * sizeof(fat::sFileEntry)) + (512 - 1)) / 512);

						mClusterRoot = mBlockRoot / mBootBlock->mBiosParams.mSectorsPerCluster;
					}
				//}
			} else {
				if (mBootBlock->mBiosParam32.mFatTotalSectors) {
					mBlockData = mBootBlock->mBiosParams.mSectorsHidden + ((mBootBlock->mBiosParam32.mFatTotalSectors * mBootBlock->mBiosParams.mFatCount) + mBootBlock->mBiosParams.mSectorsReserved);
					mBlockRoot = mBlockData + (mBlockRoot * mBootBlock->mBiosParams.mSectorsPerCluster);
				
					mClusterRoot = blockToCluster(mBlockRoot);
				}
			}

			mClustersTotal = (blockCount() - mBlockData) / mBootBlock->mBiosParams.mSectorsPerCluster;
			if (mClustersTotal < 4085)
				mType = fat::FAT12;
			else if (mClustersTotal < 65525)
				mType = fat::FAT16;
			else if (mClustersTotal <= 268435455)
				mType = fat::FAT32;

			if (mType == !clusterMapLoad()) {
				return false;
			}

			auto Root = std::make_shared<fat::sDir>(weak_from_this());
			Root->mFirstCluster = mClusterRoot;
			Root->mBlock = mBlockRoot;
			Root->mSizeInBytes = mBootBlock->mBiosParams.mRootEntryCount * sizeof(fat::sFileEntry);

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

			if (mType == fat::FAT12) {
				auto blockFat = sourceBufferCopy(mBlockFAT * blockSize(), 4608 * 3);
				if (!blockFat)
					return false;

				for (size_t i = 0, j = 0; i < 4608; i += 3) {
					mClusterMap.push_back((blockFat->at(i) + (blockFat->at(i + 1) << 8)) & 0x0FFF);
					mClusterMap.push_back((blockFat->at(i + 1) + (blockFat->at(i + 2) << 8)) >> 4);
				}
			}
			else if (mType == fat::FAT16) {
				size_t size = (mBootBlock->mBiosParams.mSectorsPerFAT * mBootBlock->mBiosParams.mFatCount) * blockSize();
				auto blockFat = sourceBufferCopy(mBlockFAT * blockSize(), size);
				if (!blockFat)
					return false;

				for (size_t i = 0; i < size; i += 2) {
					mClusterMap.push_back((blockFat->at(i) + (blockFat->at(i + 1) << 8)) & 0xFFFF);
				}
			}
			else if (mType == fat::FAT32) {
				size_t size = (mBlockFAT + mBootBlock->mBiosParam32.mFatTotalSectors) * blockSize();
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
			} else {
				return false;
			}

			return true;
		}

		/**
		 * Read and follow a chain of clusters, starting at an LBA
		 */
		spBuffer cFAT::clusterChainReadRoot(size_t pStartBlock) {
			size_t totalbytes = directorySectors(mClusterRoot) * blockSize();
			auto buffer = std::make_shared<tBuffer>();
			buffer->resize(totalbytes);
			auto dataptr = (uint8_t*)buffer->data();
			auto sector = pStartBlock;

			while (totalbytes) {
				auto size = min(totalbytes, mBootBlock->mBiosParams.mSectorsPerCluster * blockSize());
				auto block = sourceBufferCopy(sector * blockSize(), mBootBlock->mBiosParams.mSectorsPerCluster * blockSize());

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

				auto size = min(totalbytes, mBootBlock->mBiosParams.mSectorsPerCluster * blockSize());
				auto block = sourceBufferCopy(sector * blockSize(), mBootBlock->mBiosParams.mSectorsPerCluster * blockSize());

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
		bool cFAT::entrysLoad(fat::spDir pDir) {
			size_t cluster = pDir->mFirstCluster;

			spBuffer block;
			
			if(pDir == mFsRoot && mType != fat::FAT32)
				block = clusterChainReadRoot(pDir->mBlock);
			else
				block = clusterChainRead(pDir->mFirstCluster);

			fat::sFileEntry* Entry = (fat::sFileEntry*) block->data();
			fat::sFileEntry* LastEntry = Entry + (block->size() / sizeof(fat::sFileEntry));

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
					if (typeid(*entry) == typeid(fat::sDir)) {
						if (entry->mName == "." || entry->mName == "..")
							continue;
						entrysLoad(std::dynamic_pointer_cast<fat::sDir>(entry));
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
			fat::sEntry* entry = 0;

			auto StartCluster = (uint32_t)pEntry->StartCluster;

			if (mType == fat::FAT32) {
				StartCluster |= ((uint32_t)pEntry->StartClusterHi) << 16;
			}
			if (pEntry->Attributes.directory) {
				fat::spDir Dir = std::make_shared<fat::sDir>(weak_from_this());
				result = Dir;
				entry = Dir.operator->();

				Dir->mSizeInBytes = directorySectors(StartCluster) * blockSize();
			} else {
				fat::spFile File = std::make_shared<fat::sFile>(weak_from_this());
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
			fat::spFile File = std::dynamic_pointer_cast<fat::sFile>(pFile);
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
			return (mBootBlock->mBiosParams.mSectorsTotal ? mBootBlock->mBiosParams.mSectorsTotal : mBootBlock->mBiosParams.mSectorsTotal_H);
		}

		/**
		 *
		 */
		size_t cFAT::blockSize(const tBlock pBlock) const {

			if (!mBootBlock)
				return cBlocks::blockSize(pBlock);

			return mBootBlock->mBiosParams.mBytesPerSector;
		}

		/**
		 *
		 */
		tBlock cFAT::blockToCluster(const tBlock pBlock) const {
			return ((pBlock - mBlockData) / mBootBlock->mBiosParams.mSectorsPerCluster) + 2;
		}

		/**
		 *
		 */
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
					for(auto current = block; current < block + mBootBlock->mBiosParams.mSectorsPerCluster; ++current)
						free.push_back( current );
				}
				++clusternumber;
			}
			return free;
		}

		bool cFAT::filesystemChainLoad(spFile pFile) {
			return false;
		}

		fat::spFile cFAT::filesystemEntryProcess(const uint8_t* pBuffer) {
			return {};
		}

		bool cFAT::filesystemBitmapLoad() {
			return false;
		}


	}
}
