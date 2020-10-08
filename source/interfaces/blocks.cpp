#include "firy.hpp"

namespace firy {
	namespace interfaces {

		cBlocks::cBlocks(spSource pSource) : cSource(pSource) {
			mBlockCount = 0;
			mBlockSize = 0;
		}

		tBlock cBlocks::blockCount() const {
			return mBlockCount;
		}

		size_t cBlocks::blockSize(const tBlock pBlock) const {
			return mBlockSize;
		}

		size_t cBlocks::blockOffset(const tBlock pBlock) const {
			return pBlock * blockSize(pBlock);
		}

		bool cBlocks::blockWrite(const tBlock pBlock, const spBuffer pBuffer) {
			return false;
		}

		spBuffer cBlocks::blockRead(const tBlock pBlock) {
			return sourceBufferCopy(blockOffset(pBlock), blockSize());
		}
	}
}
