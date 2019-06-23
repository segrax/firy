namespace firy {
	namespace filesystem {

		struct sFile : public sNode {
		public:

			sFile(wpFilesystem pFilesystem);

			virtual spBuffer read();


			bool mChainBroken;
			std::vector<tTrackSector> mChain;
			tSize mSizeInBytes;
		};

		typedef std::shared_ptr<filesystem::sFile> spFile;
	}
}
