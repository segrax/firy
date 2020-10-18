namespace firy {

	namespace filesystem {
		class sNode;
		class sFile;
		class sDirectory;
	}
	using spNode = std::shared_ptr<filesystem::sNode>;
	using wpNode = std::weak_ptr<filesystem::sNode>;
	using wpDirectory = std::weak_ptr<filesystem::sDirectory>;

	using spFile = std::shared_ptr<filesystem::sFile>;
	using spDirectory = std::shared_ptr<filesystem::sDirectory>;

	namespace filesystem {
		/**
		 * Provide a filesystem interface
		 */
		class cInterface : public virtual firy::helpers::cDirty {
		public:
			cInterface();

			virtual std::string filesystemNameGet() const;
			virtual void filesystemNameSet(const std::string& pName);

			virtual spNode filesystemNode(const std::string& pPath);
			virtual spDirectory filesystemPath(const std::string& pPath = "/");
			virtual spFile filesystemFile(const std::string& pPath);

			virtual spBuffer filesystemRead(spNode pFile) = 0;
			virtual bool filesystemRemove(spNode pFile) = 0;

			virtual bool filesystemCreate() = 0;
			virtual bool filesystemLoad() = 0;
			virtual bool filesystemSave() = 0;

			virtual size_t filesystemTotalBytesFree() = 0;
			virtual size_t filesystemTotalBytesMax() = 0;

		protected:

			virtual bool filesystemChainLoad(spFile pFile) = 0;
			virtual bool filesystemBitmapLoad() = 0;
			virtual bool filesystemBitmapSave() = 0;

			virtual bool filesystemSaveNative() = 0;

			spDirectory mFsRoot;
			std::string mFsPathSeperator;
			std::string mFsName;
		};
	}

	using spFilesystem = std::shared_ptr<filesystem::cInterface>;
	using wpFilesystem = std::weak_ptr<filesystem::cInterface> ;
}
