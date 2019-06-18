#include "firy.hpp"

namespace firy {
	namespace interfaces {

		cBlocks::cBlocks() {
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
	}
}
