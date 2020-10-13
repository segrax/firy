namespace firy {
	namespace images {

		/**
		 * Commodore 64: D64
		 */
		namespace d64 {

			/**
			 * File Types
			 */
			enum eFileType {
				eFileType_DEL = 0,
				eFileType_SEQ = 1,
				eFileType_PRG = 2,
				eFileType_USR = 3,
				eFileType_REL = 4,
				eFileType_UNK
			};

			/**
			 * Flags
			 */
			enum eFileFlag {

				eFileFlag_LOCKED = 0x40,
				eFileFlag_CLOSED = 0x80,	// SPLAT
			};

			/**
			 * Track Bam entry
			 */
			struct sTrackBam {
				uint8_t mFreeSectors;	// Number of free sectors on this track
				uint32_t mSectors;		// Sector 00 to 07,  08 to 15, 16 to 23

				sTrackBam() {
					mFreeSectors = 0;
					mSectors = 0;
				}
			};

			/**
			 * Directory Index pointer
			 */
			struct sDirIndex {
				tTrackSector mTS;
				size_t mOffset;

				sDirIndex() { 
					mOffset = 0; 
					mTS = { 0, 0 };
				}
			};


			/**
			 *  Information about a stored file
			 */
			struct sFile : filesystem::sFile {
				size_t mSizeInSectors;
				eFileType mType;
				uint8_t mFlags;

				sDirIndex mDirIndex;
				std::vector<tTrackSector> mChain;

				sFile(wpFilesystem pFilesystem, const std::string& pName = "");
			};

			/**
			 * SharedPointer for a D64 file
			 */
			typedef std::shared_ptr<sFile> spFile;
		};

		/**
		 * Commodore 64: Disk Image (D64)
		 */
		class cD64 : public cImageAccess<access::cTracks> {

		public:
			cD64(spSource pSource);

			virtual std::string imageType() const override {
				return "Commodore 64 Disk Image (D64)";
			}

			virtual std::vector<std::string> imageExtensions() const override {
				return { "d64" };
			}

			virtual std::string filesystemNameGet() const override;
			virtual void filesystemNameSet(const std::string& pName) override;
			virtual bool filesystemCreate() override;
			virtual bool filesystemLoad() override;
			virtual bool filesystemSave() override;
			virtual spBuffer filesystemRead(spNode pFile) override;

			d64::spFile filesystemFileCreate(const std::string& pName = "") {
				auto res = std::make_shared<d64::sFile>(weak_from_this(), pName);
				res->dirty(true);
				return res;
			}

			d64::spFile filesystemFindFreeIndex(d64::spFile pFile);

			virtual tSector sectorCount(const tTrack pTrack = 0) const override;
			virtual size_t sectorSize(const tTrack pTrack = 0) const override;

			virtual bool sectorIsFree(const tTrackSector pTS) const override;
			virtual bool sectorSet(const tTrackSector pTS, const bool pValue);

			virtual std::vector<tTrackSector> sectorsUse(const tSector pTotal) override;
			virtual bool sectorsFree(const std::vector<tTrackSector> pSectors) override;

			virtual std::vector<tTrackSector> sectorsGetFree(const tTrack pTrack = 0) const override;

		protected:
			virtual bool filesystemChainLoad(spFile pFile) override;
			virtual d64::spFile filesystemEntryLoad(spBuffer pBuffer, const size_t pOffset);
			virtual bool filesystemEntrySave(d64::spFile pFile);

			virtual bool filesystemBitmapLoad() override;
			virtual bool filesystemBitmapSave() override;

		private:
			std::vector<d64::sTrackBam> mBam;

			uint8_t		mDosVersion;
			uint16_t	mDosType;
			uint16_t	mDiskID;
			std::string mLabel;
		};

	}
}
