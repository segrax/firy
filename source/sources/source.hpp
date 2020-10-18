namespace firy {

	namespace sources {
		const size_t gMegabyte = 1048576;

		class cInterface {
		public:

			cInterface(const size_t pChunkSize = gMegabyte);

			virtual bool create(const std::string pFile) = 0;
			virtual bool open(const std::string pID) = 0;
			virtual void close() = 0;
			virtual bool save(std::string pID = "") = 0;

			virtual spBuffer chunk(const size_t pFileOffset = 0) = 0;
			virtual void chunkSizeSet(const size_t pChunkSize = gMegabyte);
			virtual bool chunkPrepare(size_t pSize);

			virtual spBuffer chunkCopyToBuffer(const size_t pOffset = 0, const size_t pSize = 0);
			virtual size_t chunkCopyToPtr(uint8_t* pTarget, const size_t pSize, const size_t pOffset = 0);
			virtual bool chunkCopyFromBuffer(const size_t pOffset, spBuffer pBuffer);
			virtual bool chunkCopyFromPtr(uint8_t* pSource, const size_t pSize, const size_t pOffset);

			virtual size_t size() const;
			virtual bool hasDirtyBuffers() const;

			/**
			 * Load an object from an offset
			 */
			template <class tBlockType> std::shared_ptr<tBlockType> objectGet(const size_t pOffset = 0) {
				std::shared_ptr<tBlockType> result = std::make_shared<tBlockType>();
				size_t size = chunkCopyToPtr((uint8_t*) result.get(), sizeof(tBlockType), pOffset);
				if (size != sizeof(tBlockType))
					return 0;
				return result;
			}

			template <class tBlockType> bool objectPut(const size_t pOffset, std::shared_ptr<tBlockType> pObject) {
				return chunkCopyFromPtr((uint8_t*)pObject.get(), sizeof(tBlockType), pOffset);
			}

			virtual std::string sourceID() const { return mSourceID; }

		protected:

			std::map<size_t, spBuffer> mBuffers;		// Loaded chunks of the image, chunked by mChunkSize
			size_t		mSourceChunkSize;				// Size of each chunk in mBuffers
			size_t		mSourceSize;					// Total size of image
			bool		mCreating;						// Is the source being created
			std::string mSourceID;
		};

		
	}

	typedef std::shared_ptr<sources::cInterface> spSource;
}