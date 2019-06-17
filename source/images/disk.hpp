namespace firy {
	namespace images {

		class cDisk : public images::cImage,
			public interfaces::cTracks,
			public filesystem::cInterface,
			public std::enable_shared_from_this<cDisk> {

		public:
			cDisk();
		};
	}
}
