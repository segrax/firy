namespace firy {

	namespace filesystem {
		struct sNode;
		struct sFile;
		struct sDirectory;
	}
	typedef std::shared_ptr<filesystem::sNode> spNode;
	typedef std::shared_ptr<filesystem::sFile> spFile;
	typedef std::shared_ptr<filesystem::sDirectory> spDirectory;

	namespace filesystem {
		/**
		 * Provide a filesystem interface
		 */
		class cInterface {
		public:
			cInterface();

			virtual std::string filesystemNameGet() const { return ""; }

			virtual spNode filesystemNode(const std::string& pPath);
			virtual spDirectory filesystemPath(const std::string& pPath);
			virtual spFile filesystemFile(const std::string& pPath);
			virtual spBuffer filesystemRead(spNode pFile) = 0;
			virtual spNode filesystemAdd(spNode pFile) { gDebug->error("not implemented"); return 0; }

			virtual bool filesystemSave() { gDebug->error("not implemented"); return false; }
			virtual bool filesystemPrepare() = 0;

			virtual bool filesystemIsDirty() const { return mDirty; }
		protected:

			/**
			 * Load the T/S chain for a file
			 */
			virtual bool filesystemChainLoad(spFile pFile) = 0;
			virtual bool filesystemBitmapLoad() = 0;
			virtual bool filesystemBitmapSave() = 0;

			spDirectory mFsRoot;
			std::string mFsPathSeperator;
			bool mDirty;
		};
	}

	typedef std::shared_ptr<filesystem::cInterface> spFilesystem;
	typedef std::weak_ptr<filesystem::cInterface>	wpFilesystem;
}
