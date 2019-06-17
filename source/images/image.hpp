namespace firy {

	namespace images {

		class cImage {
		public:

			cImage();

			bool imageOpen(const std::string pFile);
			void imageClose();

			/**
			 * Get a pointer to our buffer
			 */
			virtual uint8_t* getBufferPtr(const size_t pOffset = 0) {
				if ((mBuffer->data() + pOffset) > (mBuffer->data() + mBuffer->size()))
					return 0;

				return (mBuffer->data() + pOffset);
			}

		protected:
			spBuffer mBuffer;
		};

		
	}
}