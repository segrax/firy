namespace firy {
	namespace images {
		namespace adf {

			const size_t HT_SIZE = 72;
			const size_t BM_SIZE = 25;
			const size_t MAX_DATABLK = 72;
			const size_t MAXNAMELEN = 30;
			const size_t MAXCMMTLEN = 79;

			enum eFlags {
				FFS = 1,
				INTL = 2,
				DIRCACHE = 4
			};

			enum eType {
				UNKNOWN = 0,
				FLOPPY_DD = 1,
				FLOPPY_HD = 2,
				HARDDRIVE = 3
			};

			// These are big endian
			struct sBootBlock {
				uint8_t	 dosType[4];
				uint32_t checkSum;
				int32_t	 rootBlock;
				uint8_t	 data[500 + 512];
			};


			struct sRootBlock {
				/*000*/	int32_t	type;
				int32_t	headerKey;
				int32_t	highSeq;
				/*00c*/	int32_t	hashTableSize;
				int32_t	firstData;
				/*014*/	ULONG	checkSum;
				/*018*/	int32_t	hashTable[HT_SIZE];		/* hash table */
				/*138*/	int32_t	bmFlag;				/* bitmap flag, -1 means VALID */
				/*13c*/	int32_t	bmPages[BM_SIZE];
				/*1a0*/	int32_t	bmExt;
				/*1a4*/	int32_t	cDays; 	/* creation date FFS and OFS */
				/*1a8*/	int32_t	cMins;
				/*1ac*/	int32_t	cTicks;
				/*1b0*/	char	nameLen;
				/*1b1*/	char 	diskName[MAXNAMELEN + 1];
				char	r2[8];
				/*1d8*/	int32_t	days;		/* last access : days after 1 jan 1978 */
				/*1dc*/	int32_t	mins;		/* hours and minutes in minutes */
				/*1e0*/	int32_t	ticks;		/* 1/50 seconds */
				/*1e4*/	int32_t	coDays;	/* creation date OFS */
				/*1e8*/	int32_t	coMins;
				/*1ec*/	int32_t	coTicks;
				int32_t	nextSameHash;	/* == 0 */
				int32_t	parent;		/* == 0 */
				/*1f8*/	int32_t	extension;		/* FFS: first directory cache block */
				/*1fc*/	int32_t	secType;	/* == 1 */
			};

			struct sEntryBlock {
				/*000*/	int32_t	type;		/* T_HEADER == 2 */
				/*004*/	int32_t	headerKey;	/* current block number */
				int32_t	r1[3];
				/*014*/	uint32_t	checkSum;
				/*018*/	int32_t	hashTable[HT_SIZE];
				int32_t	r2[2];
				/*140*/	int32_t	access;	/* bit0=del, 1=modif, 2=write, 3=read */
				/*144*/	int32_t	byteSize;
				/*148*/	char	commLen;
				/*149*/	char	comment[MAXCMMTLEN + 1];
				char	r3[91 - (MAXCMMTLEN + 1)];
				/*1a4*/	int32_t	days;
				/*1a8*/	int32_t	mins;
				/*1ac*/	int32_t	ticks;
				/*1b0*/	char	nameLen;
				/*1b1*/	char	name[MAXNAMELEN + 1];
				int32_t	r4;
				/*1d4*/	int32_t	realEntry;
				/*1d8*/	int32_t	nextLink;
				int32_t	r5[5];
				/*1f0*/	int32_t	nextSameHash;
				/*1f4*/	int32_t	parent;
				/*1f8*/	int32_t	extension;
				/*1fc*/	int32_t	secType;
			};

			struct sFileHeaderBlock {
				/*000*/	int32_t	type;		/* == 2 */
				/*004*/	int32_t	headerKey;	/* current block number */
				/*008*/	int32_t	highSeq;	/* number of data block in this hdr block */
				/*00c*/	int32_t	dataSize;	/* == 0 */
				/*010*/	int32_t	firstData;
				/*014*/	ULONG	checkSum;
				/*018*/	int32_t	dataBlocks[MAX_DATABLK];
				/*138*/	int32_t	r1;
				/*13c*/	int32_t	r2;
				/*140*/	int32_t	access;	/* bit0=del, 1=modif, 2=write, 3=read */
				/*144*/	uint32_t	byteSize;
				/*148*/	char	commLen;
				/*149*/	char	comment[MAXCMMTLEN + 1];
				char	r3[91 - (MAXCMMTLEN + 1)];
				/*1a4*/	int32_t	days;
				/*1a8*/	int32_t	mins;
				/*1ac*/	int32_t	ticks;
				/*1b0*/	char	nameLen;
				/*1b1*/	char	fileName[MAXNAMELEN + 1];
				int32_t	r4;
				/*1d4*/	int32_t	real;		/* unused == 0 */
				/*1d8*/	int32_t	nextLink;	/* link chain */
				int32_t	r5[5];
				/*1f0*/	int32_t	nextSameHash;	/* next entry with sane hash */
				/*1f4*/	int32_t	parent;		/* parent directory */
				/*1f8*/	int32_t	extension;	/* pointer to extension block */
				/*1fc*/	int32_t	secType;	/* == -3 */
			};

			struct sFileExtBlock {
				/*000*/	int32_t	type;		/* == 0x10 */
				/*004*/	int32_t	headerKey;
				/*008*/	int32_t	highSeq;
				/*00c*/	int32_t	dataSize;	/* == 0 */
				/*010*/	int32_t	firstData;	/* == 0 */
				/*014*/	ULONG	checkSum;
				/*018*/	int32_t	dataBlocks[MAX_DATABLK];
				int32_t	r[45];
				int32_t	info;		/* == 0 */
				int32_t	nextSameHash;	/* == 0 */
				/*1f4*/	int32_t	parent;		/* header block */
				/*1f8*/	int32_t	extension;	/* next header extension block */
				/*1fc*/	int32_t	secType;	/* -3 */
			};

			struct sOFSDataBlock {
				/*000*/	int32_t	type;		/* == 8 */
				/*004*/	int32_t	headerKey;	/* pointer to file_hdr block */
				/*008*/	int32_t	seqNum;	/* file data block number */
				/*00c*/	int32_t	dataSize;	/* <= 0x1e8 */
				/*010*/	int32_t	nextData;	/* next data block */
				/*014*/	ULONG	checkSum;
				/*018*/	UCHAR	data[488];
				/*200*/
			};

			struct sBitmapBlock {
				/*000*/	ULONG	checkSum;
				/*004*/	ULONG	map[127];
			};


			struct sBitmapExtBlock {
				/*000*/	int32_t	bmPages[127];
				/*1fc*/	int32_t	nextBlock;
			};

			struct sEntry {
				int mType;
				size_t mSizeInSectors;
				tBlock mBlock;
				tBlock mReal;
				tBlock mParent;
				tBlock mNextSameHash;
				std::string mComment;
				int32_t access;
				int year, month, days;
				int hour, mins, secs;

				sEntry() {
					mType = 0;
					mSizeInSectors = 0;
					mBlock = 0;
					mReal = 0;
					mParent = 0;
					mNextSameHash = 0;
					access = -1;
					year = month = days = hour = mins = secs = 0;
				}
			};

			/**
			 * Amiga: Representation of a file on a disk
			 */
			struct sADFFile : public sEntry, public filesystem::sFile {
				sADFFile(wpFilesystem pFilesystem);
			};

			struct sADFDir : public sEntry, public filesystem::sDirectory {
				sADFDir(wpFilesystem pFilesystem);
			};

			typedef std::shared_ptr<sADFFile> spADFFile;
			typedef std::shared_ptr<sADFDir> spADFDir;
		}

		/**
		 * Amiga: Disk Image (adf)
		 */
		class cADF : public cDisk<interfaces::cBlocks> {

		public:
			cADF();

			virtual std::string filesystemNameGet() const;
			virtual bool filesystemPrepare();
			virtual spBuffer filesystemRead(spNode pFile);

			virtual size_t blockSize(const tBlock pBlock = 0) const;
			virtual bool blockIsFree(const tBlock pBlock) const;
			virtual std::vector<tBlock> blocksFree() const;

			virtual std::shared_ptr<adf::sOFSDataBlock> blockReadOFS(const tBlock pBlock);

			adf::eType diskType() const;

		protected:
			virtual uint32_t blockBootChecksum(const uint8_t* pBuffer, const size_t pBufferLen);
			virtual uint32_t blockChecksum(const uint8_t* pBuffer, const size_t pBufferLen, const size_t pChecksumByte = 20);
			virtual bool filesystemChainLoad(spFile pFile);
			virtual bool filesystemBitmapLoad();

		private:
			template <class tBlockType> std::shared_ptr<tBlockType> blockLoad(const size_t pBlock);
			template <class tBlockType> std::shared_ptr<tBlockType> blockLoadNoCheck(const size_t pBlock);

			template <class tBlockType> void blockSwapEndian(std::shared_ptr<tBlockType> pBlock);

			bool blockBootLoad();
			bool blockRootLoad();

			spNode entryLoad(const tBlock pOffset);
			bool entrysLoad(adf::spADFDir pNode);

			std::shared_ptr<adf::sBootBlock> mBootBlock;
			std::shared_ptr<adf::sRootBlock> mRootBlock;

			tBlock mBlockFirst;
			tBlock mBlockLast;
			tBlock mBlockRoot;
			std::vector<std::shared_ptr<adf::sBitmapBlock>> mBitmapBlocks;
		};


	}
}
