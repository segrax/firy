namespace firy {

	namespace filesystem {
		struct sDirectory : public sNode {
			std::vector<spNode> mNodes;

			sDirectory(wpFilesystem pFilesystem, const std::string& pName = "");

			virtual spNode getByName(const std::string& pName, const bool pCaseSensitive = false) override;
			virtual bool add(spNode pNode) {
				mNodes.push_back(pNode);
				mFilesystem.lock()->dirty();
				dirty();

				return mFilesystem.lock()->filesystemSave();
			}
		};
	}

	typedef std::shared_ptr<filesystem::sDirectory> spDirectory;
}
