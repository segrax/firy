namespace firy {
	namespace images {
		namespace fat {

			enum eType {
				FAT12,
				FAT16,
				FAT32
			};

#pragma pack(1)

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
				uint8_t code[420];
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
				sFileAttribute Attributes;
				uint8_t Reserved[10];
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

			tBlock getFirstDataSector() const;
			tBlock fatSectorNext(tBlock pCurrent) const;
			tBlock directorySectors(tBlock pStart) const;
			tBlock clusterToBlock(tBlock pCluster) const;

			spNode entryLoad(const fat::sFileEntry* pEntry, const tBlock pBlock);

			bool entrysLoad(spFATDir pDir);

			std::shared_ptr<fat::sBootRecordBlock> mBootBlock;

			fat::eType mType;
			
			tBlock mBlockFAT;
			tBlock mBlockRoot;
			tBlock mBlockData;

			std::vector<uint16_t> mClusterMap;
		};
	}
}
