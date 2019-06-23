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
				return (mBootBlock->mRootEntCnt * sizeof(fat::sFileEntry)) / blockSize();

			cl = pStart;
			while (pStart) {
				pStart = fatSectorNext(pStart);

				if(pStart)
					totalSectors++;
			}

			return totalSectors;
		}

		tBlock cFAT::clusterToBlock(tBlock pCluster) const {
			auto dir_start = mBootBlock->mNumFATs * mBootBlock->mFATSz16 + 1;
			auto data_start = dir_start + (mBootBlock->mRootEntCnt * sizeof(sFileEntry) / mBootBlock->mBytsPerSec);
			return data_start + ((pCluster - 2) * mBootBlock->mSecPerClus);
		}

		/**
		 * Maximum number of bytes which can be stored in a block
		 */
		const size_t gBytesPerBlock = 512;

		cFAT::cFAT() : cDisk<interfaces::cBlocks>() {
			mBlockSize = gBytesPerBlock;
		}

		std::string cFAT::filesystemNameGet() const {
			return "";
		}

		bool cFAT::filesystemPrepare() {
			mBootBlock = blockLoad<sBootBlock12>(0);
			if (!mBootBlock)
				return false;

			mBlockFAT = mBootBlock->mRsvdSecCnt;

			// FAT12/16 this is a sector number
			mBlockRoot = ((tBlock) mBootBlock->mRsvdSecCnt) + uint32_t(mBootBlock->mNumFATs * mBootBlock->mFATSz16);

			auto Root = std::make_shared<sFATDir>(weak_from_this());
			Root->mBlock = mBlockRoot;
			Root->mSizeInBytes = mBootBlock->mRootEntCnt * sizeof(fat::sFileEntry);

			mFsRoot = Root;
			mClusterMap.clear();

			auto blockFat = getBufferPtr(mBlockFAT * blockSize());
			for (size_t i = 0, j = 0; i < 4608; i += 3) {
				mClusterMap.push_back((blockFat[i] + (blockFat[i + 1] << 8)) & 0x0FFF);
				mClusterMap.push_back((blockFat[i + 1] + (blockFat[i + 2] << 8)) >> 4);
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
				auto size = min(totalbytes, mBootBlock->mSecPerClus * blockSize());
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
			return mBootBlock->mTotSec16;
		}

		size_t cFAT::blockSize(const tBlock pBlock) const {
			if (!mBootBlock)
				return cBlocks::blockSize(pBlock);

			return mBootBlock->mBytsPerSec;
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
					for(auto current = block; current < block + mBootBlock->mSecPerClus; ++current)
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
