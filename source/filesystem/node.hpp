namespace firy {
	namespace filesystem {

		struct sNode : public std::enable_shared_from_this<sNode> {
			wpFilesystem mFilesystem;
			std::string mName;

			sNode(wpFilesystem pFilesystem);
			virtual spNode getByName(const std::string& pName, const bool pCaseSensitive = false);
		};
	}

}