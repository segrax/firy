namespace firy {

	typedef size_t tBlock;

	namespace interfaces {

		/**
		 * Provide a block read/write interface
		 */
		class cBlocks {
		public:
			cBlocks();

			virtual spBuffer blockRead(const tBlock pBlock) = 0;
			virtual bool blockWrite(const tBlock pBlock, const spBuffer pBuffer) = 0;

			/**
			 * Number of sectors per track
			 */
			virtual tBlock blockCount() const;

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
