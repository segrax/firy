namespace firy {

	namespace filesystem {
		struct sDirectory : public sNode {
			std::vector<spNode> mNodes;

			sDirectory(wpFilesystem pFilesystem);

			virtual spNode getByName(const std::string& pName, const bool pCaseSensitive = false) override;
			virtual spNode add(spNode pNode) {
				mNodes.push_back(pNode);
				mDirty = true;
				return pNode;
			}
		};
	}

	typedef std::shared_ptr<filesystem::sDirectory> spDirectory;
}
