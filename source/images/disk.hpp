namespace firy {
	namespace images {

		template <class tAccessInterface> class cDisk : 
			public tAccessInterface,
			public filesystem::cInterface,
			public std::enable_shared_from_this<cDisk<tAccessInterface>> {

		public:

			cDisk(spSource pSource) :
				tAccessInterface(pSource),
				cInterface(),
				std::enable_shared_from_this<cDisk<tAccessInterface>>() {

			};

			virtual std::string filesystemNameGet() const {
				return "";
			}
		};
	}
}
