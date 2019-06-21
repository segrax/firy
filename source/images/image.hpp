namespace firy {

	namespace images {

		class cImage {
		public:

			cImage();

			virtual bool imageOpen(const std::string pFile);
			virtual void imageClose();

			virtual std::shared_ptr<tBuffer> imageBufferCopy(const size_t pOffset = 0, const size_t pSize = 0) const = 0;

			/**
			 * Get a pointer to our buffer
			 */
			virtual uint8_t* getBufferPtr(const size_t pOffset = 0) const {
				if ((mBuffer->data() + pOffset) > (mBuffer->data() + mBuffer->size()))
					return 0;

				return (mBuffer->data() + pOffset);
			}

			/**
			 * Return a shared_ptr to an object
			 */
			template <class tType> std::shared_ptr<tType> imageObjectGet(const size_t pOffset = 0) const {
				auto Buffer = getBufferPtr(pOffset);
				std::shared_ptr<tType> ret = std::make_shared<tType>();

				memcpy(ret.get(), Buffer, sizeof(tType));
				return ret;
			}

		protected:
			spBuffer mBuffer;
		};

		
	}
}