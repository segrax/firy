namespace firy {

	namespace access {

		/**
		 * Provide function helpers to an underlying source
		 */
		class cInterface : public virtual firy::helpers::cDirty {

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
				dirty(false);
				return true;
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
			 * Load a specific object from an offset in the source
			 */
			template <class tBlockType> std::shared_ptr<tBlockType> objectGet(const size_t pOffset = 0) {
				return mSource->objectGet<tBlockType>(pOffset);
			}

			/**
			 * Save a specific object to an offset in the source
			 */
			template <class tBlockType> bool objectPut(const size_t pOffset, std::shared_ptr<tBlockType> pObject) {
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
				dirty(true);
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
				
				dirty(true);
				return true;
			}

			/**
			 * Get a pointer to the chunk buffer
			 * NOTE: This function is not safe, it will not cross the source chunk boundary
			 */
			uint8_t* sourceChunkPtr(const size_t pOffset = 0) {
				return mSource->sourceChunkPtr(pOffset);
			}

		protected:
			spSource mSource;

		};
	}
}
