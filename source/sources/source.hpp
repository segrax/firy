namespace firy {

	namespace sources {
		const size_t gMegabyte = 1048576;

		class cSource {
		public:

			cSource(const size_t pChunkSize = gMegabyte);

			virtual bool open(const std::string pID) = 0;
			virtual void close() = 0;

			virtual size_t size() const { return mSourceSize; };

			virtual spBuffer bufferCopy(const size_t pOffset = 0, const size_t pSize = 0) = 0;

			template <class tBlockType> std::shared_ptr<tBlockType> sourceObjectGet(const size_t pOffset = 0) {
				auto buffer = bufferCopy(pOffset, sizeof(tBlockType));
				std::shared_ptr<tBlockType> result = std::make_shared<tBlockType>();

				if (!buffer || buffer->size() < sizeof(tBlockType))
					return 0;

				memcpy(result.get(), buffer->data(), buffer->size());
				return result;
			}

			virtual spBuffer bufferChunk(const size_t pFileOffset = 0) = 0;

			/**
			 * Get a pointer to our buffer
			 */
			virtual uint8_t* sourceBufferPtr(const size_t pOffset = 0) {
				spBuffer buffer = bufferChunk(pOffset);

				return (buffer->data() + (pOffset % mSourceChunkSize));
			}

		protected:
			std::map<size_t, spBuffer> mBuffers;		// Loaded chunks of the image, chunked by mChunkSize
			size_t		mSourceChunkSize;				// Size of each chunk in mBuffers
			size_t		mSourceSize;						// Total size of image
		};

		
	}

	typedef std::shared_ptr<sources::cSource> spSource;
}