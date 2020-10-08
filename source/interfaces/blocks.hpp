namespace firy {

	typedef size_t tBlock;

	namespace interfaces {

		/**
		 * Provide a block read/write interface
		 */
		class cBlocks : public interfaces::cSource {
		public:
			cBlocks(spSource pSource);

			virtual spBuffer blockRead(const tBlock pBlock);

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
			 * Get free blocks
			 */
			virtual std::vector<tBlock> blocksFree() const = 0;

			/**
			 * Number of bytes per block
			 */
			virtual size_t blockSize(const tBlock pBlock = 0) const = 0;

			/**
			 * Load an object from a block
			 */
			template <class tBlockType> std::shared_ptr<tBlockType> blockObjectGet(const size_t pBlock) {
				return sourceObjectGet<tBlockType>(pBlock * blockSize());
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
