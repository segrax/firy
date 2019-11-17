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
			uint16_t next = mClusterMap[pCurrent];
			if ((next == 0) || (next >= 0x0FF0))
				return 0;
			
			return next;
		}

		tBlock cFAT::directorySectors(tBlock pStart) const {
			tBlock totalSectors = 1, cl;

			// The root directory (start=0) has a fixed size.
			if (!pStart)
				return (mBootBlock->mBiosParams.rootentries * sizeof(fat::sFileEntry)) / blockSize();

			cl = pStart;
			while (pStart) {
				pStart = fatSectorNext(pStart);

				if(pStart)
					totalSectors++;
			}

			return totalSectors;
		}

		tBlock cFAT::getFirstDataSector() const {
			return (mBootBlock->mBiosParam32.fatsize * mBootBlock->mBiosParams.numfats) + mBootBlock->mBiosParams.reserved;
		}

		tBlock cFAT::clusterToBlock(tBlock pCluster) const {
			int FATSz;
			if (mType == FAT32) {
				auto data_start = getFirstDataSector() + mBootBlock->mBiosParams.hidden;
				return data_start + ((pCluster - 2) * mBootBlock->mBiosParams.secperclus);
			}

			auto dir_start = mBootBlock->mBiosParams.numfats * mBootBlock->mBiosParams.secperfat + (mType == FAT32 ? 1 : 0);
			auto data_start = dir_start + (mBootBlock->mBiosParams.rootentries * sizeof(sFileEntry) / mBootBlock->mBiosParams.bytepersec);
			return data_start + ((pCluster - 2) * mBootBlock->mBiosParams.secperclus);
		}

		/**
		 * Maximum number of bytes which can be stored in a block
		 */
		const size_t gBytesPerBlock = 512;

		cFAT::cFAT() : cDisk<interfaces::cBlocks>() {
			mBlockSize = gBytesPerBlock;
			mBlockFAT = 0;
			mBlockRoot = 0;
			mType = eType::FAT12;
		}

		std::string cFAT::filesystemNameGet() const {
			return "";
		}

		bool cFAT::filesystemPrepare() {
			mBootBlock = blockLoad<sBootRecordBlock>(0);
			if (!mBootBlock)
				return false;

			mBlockFAT = mBootBlock->mBiosParams.reserved;

			// Calculate the FAT type based on cluster count
			auto numclusters = (mBootBlock->mBiosParams.sectors_s - mBlockData) / mBootBlock->mBiosParams.secperclus;
			if (numclusters < 4085)
				mType = FAT12;
			else if (numclusters < 65525)
				mType = FAT16;
			else
				mType = FAT32;

			// rootentries is set for fat12/16
			if (mBootBlock->mBiosParams.rootentries && mType != FAT32) {
				mBlockRoot = mBlockFAT + (mBootBlock->mBiosParams.secperfat * 2);
				mBlockData = mBlockRoot + (((mBootBlock->mBiosParams.rootentries * sizeof(fat::sFileEntry)) + (512 - 1)) / 512);
			} else {
				mBlockData = mBlockFAT + (mBootBlock->mBiosParams.secperfat * 2);
				mBlockRoot = mBootBlock->mBiosParam32.root;
			}

			auto Root = std::make_shared<sFATDir>(weak_from_this());
			Root->mBlock = mBlockRoot;
			Root->mSizeInBytes = mBootBlock->mBiosParams.rootentries * sizeof(fat::sFileEntry);
			//if (mType == FAT32)
			//	Root->mBlock = mBlockData + ((mBlockRoot - 2) * mBootBlock->mBiosParams.secperclus);

			mFsRoot = Root;
			mClusterMap.clear();

			auto blockFat = imageBufferCopy(mBlockFAT * blockSize(), 4608 * 3);
			if (!blockFat)
				return false;

			for (size_t i = 0, j = 0; i < 4608; i += 3) {
				mClusterMap.push_back((blockFat->at(i) + (blockFat->at(i + 1) << 8)) & 0x0FFF);
				mClusterMap.push_back((blockFat->at(i + 1) + (blockFat->at(i + 2) << 8)) >> 4);
			}

			return entrysLoad(Root);
		}

		bool cFAT::entrysLoad(spFATDir pDir) {
			auto block = imageBufferCopy(pDir->mBlock * blockSize(), pDir->mSizeInBytes);
			
			sFileEntry* Entry = (sFileEntry*) block->data();
			sFileEntry* LastEntry = Entry + (pDir->mSizeInBytes / sizeof(fat::sFileEntry));

			for (; Entry != LastEntry; ++Entry) {
				if (!Entry->Name[0])
					break;

				if (Entry->Name[0] == 0x20 || Entry->Name[0] > 0x80)
					continue;

				auto entry = entryLoad(Entry, 0);
				if (entry) {
					if (typeid(*entry) == typeid(sFATDir)) {
						if (entry->mName == "." || entry->mName == "..")
							continue;
						else
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

			if (pEntry->Attributes.directory) {
				spFATDir Dir = std::make_shared<sFATDir>(weak_from_this());
				result = Dir;
				entry = Dir.operator->();


				Dir->mSizeInBytes = directorySectors(clusterToBlock(pEntry->StartCluster)) * blockSize();
			} else {
				spFATFile File = std::make_shared<sFATFile>(weak_from_this());
				result = File;
				entry = File.operator->();

				File->mSizeInBytes = pEntry->FileLength;
			}

			entry->mFirstCluster = pEntry->StartCluster;
			entry->mBlock = clusterToBlock(pEntry->StartCluster);
			result->mName.append((const char*)pEntry->Name, 8);
			result->mName = rtrim(result->mName, 0x20);	// Trim spaces

			if (!pEntry->Attributes.directory) {
				result->mName.append(".");
				result->mName.append((const char*)pEntry->Extension, 3);
				result->mName = rtrim(result->mName, 0x20);	// Trim spaces
			}

			return result;
		}

		spBuffer cFAT::filesystemRead(spNode pFile) {
			spFATFile File = std::dynamic_pointer_cast<sFATFile>(pFile);
			if (!File)
				return {};

			tBlock cluster = File->mFirstCluster;
			size_t totalbytes = File->mSizeInBytes;
			auto buffer = std::make_shared<tBuffer>();
			buffer->resize(totalbytes);
			auto destptr = buffer->data();

			while (totalbytes) {
				auto sector = clusterToBlock(cluster);

				auto block = getBufferPtr(sector * blockSize());
				auto size = min(totalbytes, mBootBlock->mBiosParams.secperclus * blockSize());
				memcpy(destptr, block, size);
				totalbytes -= size;
				destptr += size;

				cluster = fatSectorNext(cluster);
				if (!cluster && totalbytes) {
					// TODO: Error
					return {};
				}
			}

			return buffer;
		}

		tBlock cFAT::blockCount() const {
			return mBootBlock->mBiosParams.sectors_l;
		}

		size_t cFAT::blockSize(const tBlock pBlock) const {
			if (mType == FAT32)
				return 512;

			if (!mBootBlock)
				return cBlocks::blockSize(pBlock);

			return mBootBlock->mBiosParams.bytepersec;
		}

		bool cFAT::blockIsFree(const tBlock pBlock) const {
			return false;
		}

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
