namespace firy {
	namespace filesystem {

		struct sFile : public sNode {
		public:

			sFile(wpFilesystem pFilesystem, const std::string& pName = "");

			virtual spBuffer read();

			bool mChainBroken;
			tSize mSizeInBytes;
			spBuffer mContent;
		};

		typedef std::shared_ptr<filesystem::sFile> spFile;
	}
}
