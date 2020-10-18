namespace firy {

	namespace filesystem {
		class sDirectory : public sNode {
		public:
			sDirectory(wpFilesystem pFilesystem, const std::string& pName = "");

			virtual spNode getByName(const std::string& pName, const bool pCaseSensitive = false) override;

			virtual void nodeAdd(spNode pNode);
			virtual void nodeRemove(spNode pNode);

			virtual bool add(spNode pNode);			// Add to the file system
			virtual bool remove() override;			// Remove from the file system
			virtual bool isDirectory() const override { return true; }

			std::vector<spNode> mNodes;
		};
	}

	using spDirectory = std::shared_ptr<filesystem::sDirectory>;
}
