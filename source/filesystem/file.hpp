namespace firy {
	namespace filesystem {

		struct sFile : public sNode {
		public:

			sFile(wpFilesystem pFilesystem);

			virtual spBuffer read();

			bool mChainBroken;
			tSize mSizeInBytes;
			tBuffer mContent;
		};

		typedef std::shared_ptr<filesystem::sFile> spFile;
	}
}
