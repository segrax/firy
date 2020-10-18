namespace firy {
	namespace filesystem {

		class sNode : public firy::helpers::cDirty, public std::enable_shared_from_this<sNode> {
			friend class sDirectory;
			friend class sFile;

		public:
			sNode(wpFilesystem pFilesystem, const std::string& pName = "");
			virtual spNode getByName(const std::string& pName, const bool pCaseSensitive = false);
		
			inline std::string nameGet() const { return mName; }
			inline void nameSet(const std::string pName) { mName = pName; dirty(); }

			virtual size_t sizeInBytesGet() { return mSizeInBytes; }
			virtual void sizeInBytesSet(const size_t pSizeInBytes) { mSizeInBytes = pSizeInBytes; dirty(); }

			virtual helpers::sDateTime timeAccessGet() { return mTimeAccess; }
			virtual helpers::sDateTime timeCreateGet() { return mTimeCreate; }
			virtual helpers::sDateTime timeWriteGet() { return mTimeWrite; }

			virtual void timeAccessSet(helpers::sDateTime& pTime) { mTimeAccess = pTime; dirty(); }
			virtual void timeCreateSet(helpers::sDateTime& pTime) { mTimeCreate = pTime; dirty(); }
			virtual void timeWriteSet(helpers::sDateTime& pTime) { mTimeWrite = pTime; dirty();  }

			virtual bool remove();
			
			virtual bool isDirectory() const { return false; }

		protected:
			wpFilesystem mFilesystem;

			std::string mName;
			tSize mSizeInBytes;
			helpers::sDateTime mTimeCreate;
			helpers::sDateTime mTimeWrite;
			helpers::sDateTime mTimeAccess;

			wpDirectory mParent;
		};
	}

}
