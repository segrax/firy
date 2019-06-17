namespace firy {

	typedef size_t tBlock;

	namespace interfaces {

		/**
		 * Provide a block read/write interface
		 */
		class cBlocks {
		public:
			virtual spBuffer trackRead(const tBlock pBlock) = 0;
			virtual bool trackWrite(const tBlock pBlock, const spBuffer pBuffer) = 0;
		};

	}
}
