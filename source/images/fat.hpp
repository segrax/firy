namespace firy {
	namespace images {
		namespace fat {
#pragma pack(1)
			struct sBootBlock12 {
				uint8_t mjmpBoot[3];
				uint8_t mOEMName[8];
				uint16_t mBytsPerSec;
				uint8_t  mSecPerClus;
				uint16_t mRsvdSecCnt;
				uint8_t  mNumFATs;
				uint16_t mRootEntCnt;
				uint16_t mTotSec16;		// Total blocks
				uint8_t  mMedia;
				uint16_t mFATSz16;		// Logical blocks per FAT

				uint16_t mSecPerTrk;
				uint16_t mNumHeads;
				uint32_t mHiddSec;
				uint32_t mTotSec32;
				uint8_t mData[472];
			};

			struct sBootBlock16 {
				uint8_t mjmpBoot[3];
				int8_t mOEMName[8];
				uint16_t mBytsPerSec;
				uint8_t mSecPerClus;
				uint16_t mRsvdSecCnt;
				uint8_t mNumFATs;
				uint16_t mRootEntCnt;
				uint16_t mTotSec16;
				uint8_t mMedia;
				uint16_t mFATSz16;
				uint16_t mSecPerTrk;
				uint16_t mNumHeads;
				uint32_t mHiddSec;
				uint32_t mTotSec32;

				uint8_t mDrvNum;
				uint8_t mReserved1;
				uint8_t mBootSig;
				uint32_t mVolID;
				int8_t mVolLab[11];
				int8_t  mFilSysType[8];
			};

			struct sFileEntry {
				uint8_t Name[8];
				uint8_t Extension[3];
				uint8_t Attribute;
				uint8_t Reserved[10];
				uint16_t Time;
				uint16_t Date;
				uint16_t StartCluster;
				uint32_t FileLength;
			};
		}
#pragma pack()

		/**
		 * PC: Representation of a file
		 */
		struct sFATFile : filesystem::sFile {
			size_t mSizeInSectors;

			sFATFile(wpFilesystem pFilesystem);
			spBuffer read();
		};

		typedef std::shared_ptr<sFATFile> spFATFile;

		/**
		 * MS-DOS: FAT
		 */
		class cFAT : public cDisk<interfaces::cBlocks> {

		public:
			cFAT();

			virtual std::string filesystemNameGet() const;
			virtual bool filesystemPrepare();
			virtual spBuffer filesystemRead(spNode pFile);

			virtual tBlock blockCount() const;
			virtual size_t blockSize(const tBlock pBlock = 0) const;
			virtual bool blockIsFree(const tBlock pBlock) const;
			virtual std::vector<tBlock> blocksFree() const;

		protected:
			virtual bool filesystemChainLoad(spFile pFile);
			virtual spFATFile filesystemEntryProcess(const uint8_t* pBuffer);
			virtual bool filesystemBitmapLoad();

		private:

			std::shared_ptr<fat::sBootBlock12> mBootBlock;

			tBlock mBlockFAT;
			tBlock mBlockRoot;
		};


	}
}
