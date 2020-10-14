namespace firy {
	namespace images {
		namespace adf {

			void convertDaysToDate(int32_t days, int* yy, int* mm, int* dd);

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
				eType_Unknown = 0,
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
				/*000*/	int32_t	mType;
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
				/*000*/	int32_t	mType;		/* T_HEADER == 2 */
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
				/*1f0*/	int32_t	nextSameHash;	// Block no of filename with same hash, but different file
				/*1f4*/	int32_t	parent;
				/*1f8*/	int32_t	extension;
				/*1fc*/	int32_t	secType;
			};

			struct sFileHeaderBlock {
				/*000*/	int32_t	mType;		/* == 2 */
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
				/*000*/	int32_t	mType;		/* == 0x10 */
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
				/*000*/	int32_t	mType;		/* == 8 */
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

			struct sDateTime {
				int year, month, days, hour, mins, secs;

				sDateTime();
				void reset() {
					year = month = days = hour = mins = secs = 0;
				}
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
				sDateTime mDate;

				sEntry() {
					mType = 0;
					mSizeInSectors = 0;
					mBlock = 0;
					mReal = 0;
					mParent = 0;
					mNextSameHash = 0;
					access = -1;
					mDate.reset();
				}

				uint8_t toUpperIntl(const uint8_t c) {
					return (c >= 'a' && c <= 'z') || 
						   (c >= 224 && c <= 254 && c != 247) ? c - ('a' - 'A') : c;
				}

				/**
				 * Get the hash value for the provided string
				 */
				uint32_t getHashValue(const std::string& pName, const bool pInternational) {
					uint32_t hash, len;
					unsigned int i;
					uint8_t upper;

					len = hash = min((uint32_t) pName.size(), MAXNAMELEN);
					for (i = 0; i < len; i++) {
						if (pInternational)
							upper = toUpperIntl(pName[i]);
						else
							upper = toupper(pName[i]);
						hash = (hash * 13 + upper) & 0x7ff;
					}
					hash = hash % HT_SIZE;
					return(hash);
				}

				virtual uint32_t getNameHash(const bool pInternational) = 0;
			};

			/**
			 * Representation of a file
			 */
			struct sFile : public sEntry, public filesystem::sFile {
				sFile(wpFilesystem pFilesystem, const std::string& pName = "");

				/** 
				 * Get the hash of this entry
				 */
				virtual uint32_t getNameHash(const bool pInternational) {
					return getHashValue(nameGet(), pInternational);
				}
			};

			/**
			 * Representation of a directory
			 */
			struct sDir : public sEntry, public filesystem::sDirectory {
				sDir(wpFilesystem pFilesystem, const std::string& pName = "");

				/**
				 * Get the hash of this entry
				 */
				virtual uint32_t getNameHash(const bool pInternational) {
					return getHashValue(nameGet(), pInternational);
				}
			};

			typedef std::shared_ptr<sFile> spFile;
			typedef std::shared_ptr<sDir> spDir;
			typedef std::shared_ptr<sEntry> spEntry;
		}

		/**
		 * Amiga: Disk Image (adf)
		 */
		class cADF : public cImageAccess<access::cBlocks> {

		public:
			cADF(spSource pSource);

			virtual std::string imageType() const override {
				return "Amiga Disk Format (adf)";
			}

			virtual std::vector<std::string> imageExtensions() const override {
				return { "adf", "hdf" };
			}

			virtual std::string filesystemNameGet() const override;
			virtual bool filesystemLoad() override;
			virtual bool filesystemSave() override;
			virtual bool filesystemSaveNode(spNode pNode, adf::spDir pParent);
			virtual spBuffer filesystemRead(spNode pFile) override;
			virtual bool filesystemRemove(spNode pFile) override;

			adf::spFile filesystemFileCreate(const std::string& pName = "") {
				auto res = std::make_shared<adf::sFile>(weak_from_this(), pName);
				res->dirty(true);
				return res;
			}

			virtual size_t blockSize(const tBlock pBlock = 0) const override;
			virtual bool blockIsFree(const tBlock pBlock) const override;
			virtual bool blockSet(const tBlock pBlock, const bool pValue) override;

			virtual std::vector<tBlock> blockUse(const tBlock pTotal) override;
			virtual bool blocksFree(const std::vector<tBlock>& pBlocks) override;

			virtual std::vector<tBlock> blocksGetFree() const override;

			adf::eType diskType() const;

		protected:
			virtual uint32_t blockBootChecksum(const uint8_t* pBuffer, const size_t pBufferLen);
			virtual uint32_t blockChecksum(const uint8_t* pBuffer, const size_t pBufferLen, const size_t pChecksumByte = 20);
			virtual bool filesystemChainLoad(spFile pFile) override;
			virtual bool filesystemBitmapLoad() override;
			virtual bool filesystemBitmapSave() override;

		private:
			template <class tBlockType> std::shared_ptr<tBlockType> blockLoad(const size_t pBlock);
			template <class tBlockType> std::shared_ptr<tBlockType> blockLoadNoCheck(const size_t pBlock);
			template <class tBlockType> bool blockSave(const size_t pBlock, std::shared_ptr<tBlockType> pData);

			template <class tBlockType> void blockSwapEndian(std::shared_ptr<tBlockType> pBlock);

			bool blockBootLoad();
			bool blockRootLoad();

			spNode entryLoad(const tBlock pBlock);
			bool entrysLoad(adf::spDir pNode);
			int32_t entryGet(adf::spDir pDir, spNode pEntry);

		public:
			std::shared_ptr<adf::sBootBlock> mBootBlock;
			std::shared_ptr<adf::sRootBlock> mRootBlock;

			tBlock mBlockFirst;	// Block number of first
			tBlock mBlockLast;	// Block number of last
			tBlock mBlockRoot;	// Block Number of the Root
			std::vector<std::shared_ptr<adf::sBitmapBlock>> mBitmapBlocks;
		};


	}
}
