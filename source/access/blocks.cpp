/*
 *  FIRY
 *  ---------------
 *
 *  Copyright (C) 2019-2021
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

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
