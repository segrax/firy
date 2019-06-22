namespace firy {

	typedef size_t tBlock;

	namespace interfaces {

		/**
		 * Provide a block read/write interface
		 */
		class cBlocks {
		public:
			cBlocks();
			virtual std::shared_ptr<tBuffer> imageBufferCopy(const size_t pOffset = 0, const size_t pSize = 0) const = 0;

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
			 * Number of bytes per sector
			 */
			virtual size_t blockSize(const tBlock pBlock = 0) const = 0;

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
