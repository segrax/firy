namespace firy {

	typedef size_t tBlock;

	namespace access {

		/**
		 * Provide a block read/write interface
		 */
		class cBlocks : public virtual access::cInterface {
		public:
			cBlocks();

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
			virtual std::vector<tBlock> blockUse(const tBlock pTotal) = 0;

			/**
			 * Free all blocks in 'pBlocks'
			 */
			virtual bool blocksFree(const std::vector<tBlock>& pBlocks) = 0;

			/**
			 * Get free blocks
			 */
			virtual std::vector<tBlock> blocksGetFree() const = 0;

			/**
			 * Number of bytes per block
			 */
			virtual size_t blockSize(const tBlock pBlock = 0) const = 0;

			/**
			 * Load an object from a block
			 */
			template <class tBlockType> std::shared_ptr<tBlockType> blockObjectGet(const tBlock pBlock) {
				return objectGet<tBlockType>(pBlock * blockSize());
			}

			/**
			 * Save an object to a block
			 */
			template <class tBlockType> bool blockObjectPut(const tBlock pBlock, std::shared_ptr<tBlockType> pObject) {
				return objectPut<tBlockType>(pBlock * blockSize(), pObject);
			}

			template <class tBlockType> std::shared_ptr<tBlockType> blockObjectCreate() {
				return std::make_shared<tBlockType>();
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
