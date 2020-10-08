namespace firy {

	typedef size_t tBlock;

	namespace interfaces {

		class cSource {

		public:
			cSource(spSource pSource) {
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
			template <class tBlockType> std::shared_ptr<tBlockType> sourceObjectGet(const size_t pOffset = 0) {
				return mSource->sourceObjectGet<tBlockType>(pOffset);
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
				return mSource->bufferChunk(pOffset);
			}

			/**
			 * Get a pointer to the chunk buffer
			 * NOTE: This function is not safe, it will not cross the source chunk boundary
			 */
			uint8_t* sourceBufferPtr(const size_t pOffset = 0) {
				return mSource->sourceBufferPtr(pOffset);
			}

		protected:
			spSource mSource;

		};
	}
}
