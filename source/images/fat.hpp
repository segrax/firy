namespace firy {
	namespace images {
		namespace fat {

			/**
			 * Supported FAT revisions
			 */
			enum eType {
				eType_Unknown,
				eType_FAT12,
				eType_FAT16,
				eType_FAT32
			};

#pragma pack(1)
			/**
			 * On disk: Partition Table
			 */
			struct sParitionTableEntry {
				uint8_t  mActive;				// 0x80: Active/mActive partition
				uint8_t  mStartHead;
				unsigned char mStartSector: 6;
				unsigned char mStartCylinderHigh : 2;
				uint8_t  mStartCylinderLow;

				uint8_t  mType;

				uint8_t  mEndHead;
				unsigned char mEndSector : 6;
				unsigned char mEndCylinderLowHigh : 2;
				uint8_t  mEndCylinderLow;

				uint32_t mStartLBA;		// LBA of first block of partition
				uint32_t mTotalSectors;		// Total blocks
			};

			/*
			 *	BIOS Parameter Block structure (eType_FAT12/16)
			 */
			struct sBiosParam {
				uint16_t mBytesPerSector;		// bytes per sector
				uint8_t  mSectorsPerCluster;	// (1,2,4,8,16,32,64,128 are valid)
				uint16_t mSectorsReserved;		// Total of reserved sectors
				uint8_t  mFatCount;				// Count of the number of FAT tables
				uint16_t mRootEntryCount;		// Count of entries in root directory
				uint16_t mSectorsTotal;			// small num sectors
				uint8_t  mMediaType;			// media descriptor byte
				uint16_t mSectorsPerFAT;		// sectors per FAT 
				uint16_t mSectorsPerTrack;		// sectors per track
				uint16_t mHeads;				// disk heads
				uint32_t mSectorsHidden;		// hidden sectors
				uint32_t mSectorsTotal_H;		// eType_FAT32 Sectors Total
			};

			/*
			 *	Extended BIOS Parameter Block structure (eType_FAT12/16)
			 */
			struct sBiosExParam {
				uint8_t  mUnit;				// int 13h drive#
				uint8_t  mHead;				// 
				uint8_t  mSignature;		// 0x28 or 0x29
				uint32_t mSerial;			// serial#
				char	 mLabel[11];		// volume label
				uint8_t  mFilesystemID[8];	// 
			};

			/*
			 *	Extended BIOS Parameter Block structure (eType_FAT32)
			 */
			struct sBiosExParam32 {
				uint32_t mFatTotalSectors;		// big FAT size in sectors
				uint16_t mExtendedFlags;		// extended flags
				uint16_t mFilesystemVersion;	// filesystem version (0x00)
				uint32_t mRootCluster;			// cluster of root dir
				uint16_t mFilesystemInfoSector;	// sector pointer to FSINFO within reserved area
				uint16_t mBackupBootSector;		// sector pointer to backup mActive sector within reserved area
				uint8_t  mReserved[12];			// reserved, should be 0

				uint8_t  mUnit;					// int 13h drive#
				uint8_t  mHead;					// archaic, used by Windows NT-class OSes for flags
				uint8_t  mSignature;			// 0x28 or 0x29
				uint32_t mSerial;				// serial#
				uint8_t  mLabel[11];			// volume label
				uint8_t  mFilesystemID[8];		// filesystem ID
			};

			/**
			 * Boot Record
			 */
			struct sBootRecordBlock {
				uint8_t mJmpBoot[3];
				uint8_t mOEMName[8];
				sBiosParam mBiosParams;
				union {
					sBiosExParam mBiosParam;
					sBiosExParam32 mBiosParam32;
				};

				union {
					uint8_t mBootCode1[420];

					struct {
						uint8_t mBootCode2[356];
						sParitionTableEntry mPartitions[4];	// 0x88
					} mMasterBootRecord;
				};

				uint8_t mSignature1;		
				uint8_t mSignature2;	
			};

			struct sFileAttribute {
				uint8_t read_only : 1;
				uint8_t hidden : 1;
				uint8_t mFilesystemID : 1;
				uint8_t mLabel : 1;
				uint8_t directory : 1;
				uint8_t archive : 1;
				uint8_t __res : 2;
			};

			struct sFileEntry {
				uint8_t Name[8];
				uint8_t Extension[3];
				union {
					sFileAttribute Attributes;
					uint8_t Attribute;
				};

				uint8_t Reserved;
				uint8_t CrtTimeTenth;
				uint16_t CrtTime;
				uint16_t CrtDate;
				uint16_t LstAccDate;
				uint16_t StartClusterHi;	// eType_FAT32
				uint16_t Time;
				uint16_t Date;
				uint16_t StartCluster;
				uint32_t FileLength;
			};

			struct sFileLongNameEntry {
				uint8_t mSequence;
				wchar_t mName1[5];
				union {
					sFileAttribute Attributes;
					uint8_t Attribute;
				};
				uint8_t mReserved;
				uint8_t mChecksum;
				wchar_t mName2[6];
				uint16_t mReserved2;
				wchar_t mName3[2];
			};

#pragma pack()

			struct sEntry {
				tBlock mBlock;
				tBlock mFirstCluster;
				std::string mShortName;
				std::wstring mUnicodeName;
			};

			/**
			 * PC: Representation of a file
			 */
			struct sFile : public sEntry, public filesystem::sFile {
				size_t mSizeInSectors;

				sFile(wpFilesystem pFilesystem, const std::string& pName = "");
			};

			struct sDir : public sEntry, public filesystem::sDirectory {
				size_t mSizeInBytes;

				sDir(wpFilesystem pFilesystem, const std::string& pName = "");
			};

			typedef std::shared_ptr<sFile> spFile;
			typedef std::shared_ptr<sDir> spDir;
		}


		/**
		 * File Allocation Table
		 */
		class cFAT : public cImageAccess<access::cBlocks> {

		public:
			cFAT(spSource pSource);

			virtual std::string filesystemNameGet() const;
			virtual bool filesystemLoad();
			virtual spBuffer filesystemRead(spNode pFile);

			virtual bool partitionOpen(int pNumber);

			virtual tBlock blockCount() const;
			virtual size_t blockSize(const tBlock pBlock = 0) const;
			virtual tBlock blockToCluster(const tBlock pBlock) const;

			virtual bool blockIsFree(const tBlock pBlock) const;
			virtual std::vector<tBlock> blocksFree() const;

			spBuffer clusterChainReadRoot(size_t pStartBlock);
			spBuffer clusterChainRead(size_t pCluster);

			virtual std::string imageType() const {
				return "FAT";
			}

			virtual std::vector<std::string> imageExtensions() const {
				return { "img" };
			}
		protected:
			virtual bool filesystemChainLoad(spFile pFile);
			virtual bool filesystemBitmapLoad();
			virtual bool filesystemBitmapSave();

		private:
			tBlock fatSectorNext(tBlock pCluster) const;
			tBlock directorySectors(tBlock pStart) const;
			tBlock clusterToBlock(tBlock pCluster) const;
			bool clusterMapLoad();

			spNode entryLoad(const fat::sFileEntry* pEntry, std::vector<fat::sFileLongNameEntry*>& pLongEntries);

			bool entrysLoad(fat::spDir pDir);

		private:
			std::shared_ptr<fat::sBootRecordBlock> mBootBlock;

			fat::eType mType;
			
			tBlock mBlockPartitionStart;

			tBlock mBlockFAT;		// Block of the FAT
			tBlock mBlockRoot;		// Block of the Root
			tBlock mBlockData;		// Block of first regular cluster

			tBlock mClusterRoot;	// Cluster of Root
			tBlock mClustersTotal;	// Total number of clusters

			std::string mLabel;
			std::vector<uint32_t> mClusterMap;
		};
	}
}
