namespace firy {
	namespace images {

		template <class tAccessInterface> class cDisk : 
			public images::cImage,
			public tAccessInterface,
			public filesystem::cInterface,
			public std::enable_shared_from_this<cDisk<tAccessInterface>> {

		public:

			cDisk() :
				cImage(),
				tAccessInterface(),
				cInterface(),
				std::enable_shared_from_this<cDisk<tAccessInterface>>() {

			};

			virtual std::string filesystemNameGet() const {
				return "";
			}

			virtual std::shared_ptr<tBuffer> imageBufferCopy(const size_t pOffset = 0, const size_t pSize = 0) const {
				auto Buffer = getBufferPtr(pOffset);
				auto result = std::make_shared<tBuffer>();
				result->resize(pSize);
				memcpy(result.get(), Buffer, pSize);
				return result;
			}
		private:

			
		};
	}
}
