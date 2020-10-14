namespace firy {
	namespace filesystem {

		struct sNode : public firy::helpers::cDirty, public std::enable_shared_from_this<sNode> {
			friend struct sDirectory;
			friend struct sFile;

			sNode(wpFilesystem pFilesystem, const std::string& pName = "");
			virtual spNode getByName(const std::string& pName, const bool pCaseSensitive = false);
		
			inline std::string nameGet() const { return mName; }
			inline void nameSet(const std::string pName) { mName = pName; dirty(); }

			virtual bool remove();


		protected:
			wpFilesystem mFilesystem;
			std::string mName;
		};
	}

}
