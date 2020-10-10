namespace firy {

	namespace access {

		/**
		 * Provide function helpers to an underlying source
		 */
		class cInterface {

		public:
			cInterface(spSource pSource) {
				mSource = pSource;
			}

		protected:
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
			 * Return a copy of the source
			 */
			spBuffer sourceBufferCopy(const size_t pOffset = 0, const size_t pSize = 0) {
				return mSource->bufferCopy(pOffset, pSize);
			}

			/**
			 * Get a chunk from the source (based on mSourceChunkSize)
			 */
			spBuffer sourceBufferChunk(const size_t pOffset = 0) {
				return mSource->chunk(pOffset);
			}

			/**
			 * Get a pointer to the chunk buffer
			 * NOTE: This function is not safe, it will not cross the source chunk boundary
			 */
			uint8_t* chunkPtr(const size_t pOffset = 0) {
				return mSource->chunkPtr(pOffset);
			}

		protected:
			spSource mSource;

		};
	}
}
