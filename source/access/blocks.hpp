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

namespace firy {

	namespace access {

		/**
		 * Provide a block read/write interface
		 */
		class cBlocks : public virtual access::cInterface {
		public:
			cBlocks();

			/**
			 *
			 */
			virtual spBuffer blockRead(const tBlock pBlock);

			/**
			 *
			 */
			virtual bool blockWrite(const tBlock pBlock, const spBuffer pBuffer);

			/**
			 * Number of blocks
			 */
			virtual tBlock blockCount() const;

			/**
			 * Is a block free
			 */
			virtual bool blockIsFree(const tBlock pBlock) const = 0;

			/**
			 * Set a block used
			 */
			virtual bool blockSet(const tBlock pBlock, const bool pValue) = 0;

			/**
			 * Get a single block, marking it used
			 */
			virtual tBlock blockUseSingle();

			/**
			 * Get 'pTotal' number of blocks, marking them used
			 */
			virtual std::vector<sAccessUnit> blockUse(const tBlock pTotal) = 0;

			/**
			 * Free all blocks in 'pBlocks'
			 */
			virtual bool blocksFree(const std::vector<sAccessUnit>& pBlocks) = 0;

			/**
			 * Get free blocks
			 */
			virtual std::vector<sAccessUnit> blocksGetFree() const = 0;

			/**
			 * Number of bytes per block
			 */
			virtual size_t blockSize(const tBlock pBlock = 0) const = 0;

			/**
			 * Load an object from a block
			 */
			template <class tBlockType> std::shared_ptr<tBlockType> blockObjectGet(const tBlock pBlock) {
				return sourceObjectGet<tBlockType>(pBlock * blockSize());
			}

			/**
			 * Save an object to a block
			 */
			template <class tBlockType> bool blockObjectPut(const tBlock pBlock, std::shared_ptr<tBlockType> pObject) {
				return sourceObjectPut<tBlockType>(pBlock * blockSize(), pObject);
			}

			/**
			 * Create an object
			 */
			template <class tBlockType> std::shared_ptr<tBlockType> blockObjectCreate() {
				return std::make_shared<tBlockType>();
			}

			/**
			 * Get free blocks
			 */
			virtual std::vector<sAccessUnit> unitGetFree() const override {
				return blocksGetFree();
			}

			virtual spBuffer unitRead(sAccessUnit pChain) override {
				return blockRead(pChain.block());
			}

		protected:

			/**
			 * Get the offset from the start of the image, to the block
			 */
			virtual size_t blockOffset(const tBlock pBlock) const;

			/**
			 * Number of blocks
			 */
			tBlock mBlockCount;

			/**
			 * Size of a block in bytes
			 */
			size_t mBlockSize;
		};

	}
}
