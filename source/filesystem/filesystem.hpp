namespace firy {

	namespace filesystem {
		struct sNode;
		struct sFile;
		struct sDirectory;
	}
	typedef std::shared_ptr<filesystem::sNode> spNode;
	typedef std::weak_ptr<filesystem::sNode> wpNode;

	typedef std::shared_ptr<filesystem::sFile> spFile;
	typedef std::shared_ptr<filesystem::sDirectory> spDirectory;

	namespace filesystem {
		/**
		 * Provide a filesystem interface
		 */
		class cInterface : public virtual firy::helpers::cDirty {
		public:
			cInterface();

			virtual std::string filesystemNameGet() const { return ""; }
			virtual void filesystemNameSet(const std::string& pName) { return; }

			virtual spNode filesystemNode(const std::string& pPath);
			virtual spDirectory filesystemPath(const std::string& pPath = "/");
			virtual spFile filesystemFile(const std::string& pPath);

			virtual spBuffer filesystemRead(spNode pFile) = 0;
			virtual bool filesystemRemove(spNode pFile) = 0;

			virtual bool filesystemCreate() { gDebug->error("not implemented"); return false; }
			virtual bool filesystemLoad() = 0;
			virtual bool filesystemSave() { gDebug->error("not implemented"); return false; }
		protected:

			/**
			 * Load the T/S chain for a file
			 */
			virtual bool filesystemChainLoad(spFile pFile) = 0;
			virtual bool filesystemBitmapLoad() = 0;
			virtual bool filesystemBitmapSave() = 0;

			spDirectory mFsRoot;
			std::string mFsPathSeperator;
		};
	}

	typedef std::shared_ptr<filesystem::cInterface> spFilesystem;
	typedef std::weak_ptr<filesystem::cInterface>	wpFilesystem;
}
