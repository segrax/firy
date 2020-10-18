namespace firy {

	typedef size_t tTrack;
	typedef size_t tSector;
	typedef size_t tBlock;

	typedef std::pair<tTrack, tSector> tTrackSector;

	struct sChainEntry {
		tTrackSector mTS;
		tBlock mBlock;

		sChainEntry() { mTS.first = 0; mTS.second = 0; mBlock = 0; }
		sChainEntry(const tTrackSector& pTS) { mTS = pTS; }
		sChainEntry(const tBlock& pBlock) { mBlock = pBlock; }
		sChainEntry(const sChainEntry& pChain) { mTS = pChain.mTS; mBlock = pChain.mBlock; }

		void operator=(const sChainEntry& pChain) { mTS = pChain.mTS; mBlock = pChain.mBlock; }
		tTrack track() const { return mTS.first; }
		tSector sector() const { return mTS.second; }
		tBlock block() const { return mBlock; }
	};

	namespace access {

		/**
		 * Provide function helpers to an underlying source
		 */
		class cInterface {

		public:
			/**
			 * Source-Access Interface
			 *
			 * Throws if shared_ptr is empty
			 */
			cInterface(spSource pSource) {
				mSource = pSource;

				assertSource();
			}

			/**
			 * Save changes back to source
			 */
			bool sourceSave(const std::string pID = "") {
				if (!mSource->save(pID))
					return false;
				return true;
			}

			/**
			 * Get a source id
			 */
			std::string sourceID() const {
				return mSource->sourceID();
			}
		protected:

			/**
			 * Throw an exception if a source wasn't provided
			 */
			inline void assertSource() const {
				if (!mSource)
					throw std::exception("Source was not found");
			}

			/**
			 * Get the size of the source in bytes
			 */
			size_t sourceSize() const {
				return mSource->size();
			}

			/**
			 * Is the source dirty
			 */
			bool sourceIsDirty() const {
				return mSource->hasDirtyBuffers();
			}

			/**
			 * Load a specific object from an offset in the source
			 */
			template <class tBlockType> std::shared_ptr<tBlockType> sourceObjectGet(const size_t pOffset = 0) {
				return mSource->objectGet<tBlockType>(pOffset);
			}

			/**
			 * Save a specific object to an offset in the source
			 */
			template <class tBlockType> bool sourceObjectPut(const size_t pOffset, std::shared_ptr<tBlockType> pObject) {
				return mSource->objectPut<tBlockType>(pOffset, pObject);
			}

			/**
			 * Return a copy of the source
			 */
			spBuffer sourceBufferCopy(const size_t pOffset = 0, const size_t pSize = 0) {
				return mSource->chunkCopyToBuffer(pOffset, pSize);
			}

			/**
			 * Get a chunk from the source (based on mSourceChunkSize)
			 */
			spBuffer sourceBufferChunk(const size_t pOffset = 0) {
				return mSource->chunk(pOffset);
			}

			/**
			 * Write to a chunk
			 */
			bool sourceBufferWrite(const size_t pOffset, const spBuffer pBuffer) {

				if (!mSource->chunkCopyFromBuffer(pOffset, pBuffer))
					return false;
				return true;
			}

			/**
			 * Prepare chunks upto a specific size
			 *
			 * This would be used if you require a specific image size
			 *  But dont want to adjust mSourceChunkSize to be a multiple of it
			 */
			bool sourceChunkPrepare(const size_t pSize) {
				if (!mSource->chunkPrepare(pSize))
					return false;
				return true;
			}

		protected:
			spSource mSource;

		};
	}
}
