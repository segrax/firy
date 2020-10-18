namespace firy {
	namespace filesystem {

		class sFile : public sNode {
		public:

			sFile(wpFilesystem pFilesystem, const std::string& pName = "");

			virtual spBuffer read();

			bool mChainBroken;
			std::vector<sChainEntry> mChain;

			spBuffer mContent;
		};

		using spFile = std::shared_ptr<filesystem::sFile>;
	}
}
