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
		}

		/**
		 * Amiga: Disk Image (adf)
		 */
		class cADF : public cDisk<interfaces::cBlocks> {

		public:
			cADF();


			virtual bool filesystemPrepare();
			virtual spBuffer filesystemRead(spNode pFile);

			virtual spBuffer blockRead(const tBlock pBlock);
			virtual bool blockWrite(const tBlock pBlock, const spBuffer pBuffer);

			virtual size_t blockSize(const tBlock pBlock = 0) const;

			adf::eType diskType() const;

		protected:
			virtual uint32_t blockChecksum(const uint8_t* pBuffer, const size_t pBufferLen);
			virtual bool filesystemChainLoad(spFile pFile);

		private:
			bool blockBootLoad();
			bool blockRootLoad();

			bool blockRootFloppyLoad(const tBlock pSectors);
			bool blockRootHarddriveLoad();

			adf::sBootBlock mBootBlock;
			adf::sRootBlock mRootBlock;

			tBlock mBlockFirst;
			tBlock mBlockLast;
			tBlock mBlockRoot;
		};


	}
}
