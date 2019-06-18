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
		};
	}
}
