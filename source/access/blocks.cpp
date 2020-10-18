#include "firy.hpp"

namespace firy {
	namespace access {

		cBlocks::cBlocks() {
			mBlockCount = 0;
			mBlockSize = 0;
		}

		tBlock cBlocks::blockCount() const {
			return mBlockCount;
		}

		tBlock cBlocks::blockUseSingle() {
			auto blocks = blockUse(1);
			if (blocks.size() == 1) {
				return blocks[0].mBlock;
			}
			throw std::exception("not free block");
		}

		size_t cBlocks::blockSize(const tBlock pBlock) const {
			return mBlockSize;
		}

		size_t cBlocks::blockOffset(const tBlock pBlock) const {
			return pBlock * blockSize(pBlock);
		}

		bool cBlocks::blockWrite(const tBlock pBlock, const spBuffer pBuffer) {
			return sourceBufferWrite(blockOffset(pBlock), pBuffer);
		}

		spBuffer cBlocks::blockRead(const tBlock pBlock) {
			return sourceBufferCopy(blockOffset(pBlock), blockSize());
		}
	}
}
