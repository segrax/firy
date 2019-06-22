#include "firy.hpp"
#include "fat.hpp"

namespace firy {
	namespace images {

		sFATFile::sFATFile(wpFilesystem pFilesystem) : filesystem::sFile(pFilesystem) {

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
			mBootBlock = blockLoad<fat::sBootBlock12>(0);
			if (!mBootBlock)
				return false;

			mBlockFAT = mBootBlock->mRsvdSecCnt;
			mBlockRoot = mBootBlock->mRsvdSecCnt + uint32_t(mBootBlock->mNumFATs * mBootBlock->mFATSz16);

			auto block = imageBufferCopy( mBlockRoot * blockSize(), mBootBlock->mRootEntCnt * sizeof(fat::sFileEntry));

			fat::sFileEntry* Entry = (fat::sFileEntry*) block->data();
			fat::sFileEntry* LastEntry = Entry + mBootBlock->mRootEntCnt;

			for (; Entry != LastEntry; ++Entry) {
				if (!Entry->Name[0])
					break;

				std::cout << Entry->Name << "\n";

			}
			return true;
		}

		spBuffer cFAT::filesystemRead(spNode pFile) {
			return {};
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
			return {};
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
