namespace firy {

	namespace sources {
		const size_t gMegabyte = 1048576;

		class cInterface {
		public:

			cInterface(const size_t pChunkSize = gMegabyte);

			virtual bool open(const std::string pID) = 0;
			virtual void close() = 0;

			virtual spBuffer chunk(const size_t pFileOffset = 0) = 0;

			virtual spBuffer bufferCopy(const size_t pOffset = 0, const size_t pSize = 0);
			virtual size_t bufferCopyTo(uint8_t* pTarget, const size_t pSize, const size_t pOffset = 0);

			virtual size_t size() const;

			/**
			 *
			 */
			template <class tBlockType> std::shared_ptr<tBlockType> objectGet(const size_t pOffset = 0) {

				std::shared_ptr<tBlockType> result = std::make_shared<tBlockType>();
				size_t size = bufferCopyTo((uint8_t*) result.get(), sizeof(tBlockType), pOffset);
				if (size != sizeof(tBlockType))
					return 0;
				return result;
			}


			virtual uint8_t* chunkPtr(const size_t pOffset = 0);

		protected:
			std::map<size_t, spBuffer> mBuffers;		// Loaded chunks of the image, chunked by mChunkSize
			size_t		mSourceChunkSize;				// Size of each chunk in mBuffers
			size_t		mSourceSize;						// Total size of image
		};

		
	}

	typedef std::shared_ptr<sources::cInterface> spSource;
}