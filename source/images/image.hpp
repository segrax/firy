namespace firy {

	namespace images {
		const size_t gMegabyte = 1048576;

		class cImage {
		public:

			cImage(const size_t pChunkSize = gMegabyte);

			virtual bool imageOpen(const std::string pFile);
			virtual void imageClose();

			virtual std::shared_ptr<tBuffer> imageBufferCopy(const size_t pOffset = 0, const size_t pSize = 0);

			template <class tBlockType> std::shared_ptr<tBlockType> imageObjectGet(const size_t pOffset = 0) {
				auto buffer = imageBufferCopy(pOffset, sizeof(tBlockType));
				std::shared_ptr<tBlockType> result = std::make_shared<tBlockType>();

				if (buffer->size() < sizeof(tBlockType))
					return 0;

				memcpy(result.get(), buffer->data(), buffer->size());
				return result;
			}

			virtual spBuffer imageChunkBuffer(const size_t pFileOffset = 0);

			/**
			 * Get a pointer to our buffer
			 */
			virtual uint8_t* getBufferPtr(const size_t pOffset = 0) {
				spBuffer buffer = imageChunkBuffer(pOffset);

				return (buffer->data() + pOffset);
			}

		protected:
			std::map<size_t, spBuffer> mBuffers;		// Loaded chunks of the image, chunked by mChunkSize
			size_t		mImageChunkSize;				// Size of each chunk in mBuffers
			std::string mImageFilename;					// Path/Name of image file
			size_t		mImageSize;						// Total size of image
		};

		
	}
}