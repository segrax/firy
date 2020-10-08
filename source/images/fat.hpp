namespace firy {
	namespace images {
		namespace fat {

			enum eType {
				FAT12,
				FAT16,
				FAT32
			};

#pragma pack(1)
			struct sParitionTableEntry {
				uint8_t  boot;				// 0x80: Active/boot partition
				uint8_t  beginHead;
				unsigned char beginSector : 6;
				unsigned char beginCylinderHigh : 2;
				uint8_t  beginCylinderLow;

				uint8_t  type;

				uint8_t  endHead;
				unsigned char endSector : 6;
				unsigned char endCylinderHigh : 2;
				uint8_t  endCylinderLow;

				uint32_t firstSector;		// LBA of first block of partition
				uint32_t totalSectors;		// Total blocks
			};

			/*
				BIOS Parameter Block structure (FAT12/16)
			*/
			struct sBiosParam {
				uint16_t bytepersec;		// bytes per sector (0x00)
				uint8_t	secperclus;			// sectors per cluster (1,2,4,8,16,32,64,128 are valid)
				uint16_t reserved;			// reserved sectors
				uint8_t numfats;			// number of FAT copies (2)
				uint16_t rootentries;		// number of root dir entries (0x00 normally)
				uint16_t sectors_s;			// small num sectors
				uint8_t mediatype;			// media descriptor byte
				uint16_t secperfat;			// sectors per FAT 
				uint16_t secpertrk;			// sectors per track
				uint16_t heads;				// heads
				uint32_t hidden;			// hidden sectors
				uint32_t sectors_l;			// large num sectors
			};

			/*
				Extended BIOS Parameter Block structure (FAT12/16)
			*/
			struct sBiosExParam {
				uint8_t unit;				// int 13h drive#
				uint8_t head;				// archaic, used by Windows NT-class OSes for flags
				uint8_t signature;			// 0x28 or 0x29
				uint32_t serial;			// serial#
				uint8_t label[11];			// volume label
				uint8_t system[8];			// filesystem ID
			};

			/*
				Extended BIOS Parameter Block structure (FAT32)
			*/
			struct sBiosExParam32 {
				uint32_t fatsize;			// big FAT size in sectors
				uint16_t extflags;			// extended flags
				uint16_t fsver;				// filesystem version (0x00)
				uint32_t root;				// cluster of root dir
				uint16_t fsinfo;			// sector pointer to FSINFO within reserved area
				uint16_t bkboot;			// sector pointer to backup boot sector within reserved area
				uint8_t reserved[12];		// reserved, should be 0

				uint8_t unit;				// int 13h drive#
				uint8_t head;				// archaic, used by Windows NT-class OSes for flags
				uint8_t signature;			// 0x28 or 0x29
				uint32_t serial;			// serial#
				uint8_t label[11];			// volume label
				uint8_t system[8];			// filesystem ID
			};


			struct sBootRecordBlock {
				uint8_t mJmpBoot[3];
				uint8_t mOEMName[8];
				sBiosParam mBiosParams;
				union {
					sBiosExParam mBiosParam;
					sBiosExParam32 mBiosParam32;
				};

				union {
					uint8_t code1[420];

					struct {
						uint8_t code2[356];
						sParitionTableEntry parts[4];	// 0x88
					} mbr;
				};

				uint8_t sig_55;		
				uint8_t sig_aa;	
			};

			struct sFileAttribute {
				uint8_t read_only : 1;
				uint8_t hidden : 1;
				uint8_t system : 1;
				uint8_t label : 1;
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
				uint16_t StartClusterHi;	// FAT32
				uint16_t Time;
				uint16_t Date;
				uint16_t StartCluster;
				uint32_t FileLength;
			};
		}
#pragma pack()

		struct sEntry {
			tBlock mBlock;
			tBlock mFirstCluster;
		};

		/**
		 * PC: Representation of a file
		 */
		struct sFATFile : public sEntry, public filesystem::sFile {
			size_t mSizeInSectors;

			sFATFile(wpFilesystem pFilesystem);
		};

		struct sFATDir : public sEntry, public filesystem::sDirectory {
			size_t mSizeInBytes;

			sFATDir(wpFilesystem pFilesystem);
		};

		typedef std::shared_ptr<sFATFile> spFATFile;
		typedef std::shared_ptr<sFATDir> spFATDir;

		/**
		 * MS-DOS: FAT
		 */
		class cFAT : public cDisk<interfaces::cBlocks> {

		public:
			cFAT(spSource pSource);

			virtual std::string filesystemNameGet() const;
			virtual bool filesystemPrepare();
			virtual spBuffer filesystemRead(spNode pFile);

			virtual bool partitionOpen(int pNumber);

			virtual tBlock blockCount() const;
			virtual size_t blockSize(const tBlock pBlock = 0) const;
			virtual tBlock blockToCluster(const tBlock pBlock) const;

			virtual bool blockIsFree(const tBlock pBlock) const;
			virtual std::vector<tBlock> blocksFree() const;

			spBuffer clusterChainReadRoot(size_t pStartBlock);
			spBuffer clusterChainRead(size_t pCluster);

		protected:
			virtual bool filesystemChainLoad(spFile pFile);
			virtual spFATFile filesystemEntryProcess(const uint8_t* pBuffer);
			virtual bool filesystemBitmapLoad();

		private:
			tBlock fatSectorNext(tBlock pCluster) const;
			tBlock directorySectors(tBlock pStart) const;
			tBlock clusterToBlock(tBlock pCluster) const;
			bool clusterMapLoad();

			spNode entryLoad(const fat::sFileEntry* pEntry, const tBlock pBlock);

			bool entrysLoad(spFATDir pDir);

			std::shared_ptr<fat::sBootRecordBlock> mBootBlock;

			fat::eType mType;
			
			tBlock mBlockPartitionStart;

			tBlock mBlockFAT;		// Block of the FAT
			tBlock mBlockRoot;		// Block of the Root
			tBlock mBlockData;		// Block of first regular cluster

			tBlock mClusterRoot;	// Cluster of Root
			tBlock mClustersTotal;	// Total number of clusters

			std::vector<uint32_t> mClusterMap;
		};
	}
}
