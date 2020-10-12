namespace firy {
	namespace images {
		namespace d64 {

			enum eFileType {
				eFileType_DEL = 0,
				eFileType_SEQ = 1,
				eFileType_PRG = 2,
				eFileType_USR = 3,
				eFileType_REL = 4,
				eFileType_UNK
			};

			struct sDirIndex {
				tTrackSector mTS;
				size_t mOffset;

				sDirIndex() { 
					mOffset = 0; 
					mTS = { 0, 0 };
				}
			};

			/**
			 * Commodore 64: Representation of a file on a disk
			 */
			struct sFile : filesystem::sFile {
				size_t mSizeInSectors;
				eFileType mType;
				sDirIndex mDirIndex;
				std::vector<tTrackSector> mChain;

				sFile(wpFilesystem pFilesystem);
			};

			/**
			 * Commodore 64: Track Bam entry
			 */
			struct sTrackBam {
				uint8_t mFreeSectors;	// Number of free sectors on this track
				uint32_t mSectors;		// Sector 00 to 07,  08 to 15, 16 to 23
			};

			typedef std::shared_ptr<sFile> spFile;
		};

		/**
			* Commodore 64: Disk Image (D64)
			*/
		class cD64 : public cImageAccess<access::cTracks> {

		public:
			cD64(spSource pSource);

			virtual std::string filesystemNameGet() const;
			virtual bool filesystemPrepare();
			virtual spBuffer filesystemRead(spNode pFile);
			virtual spNode filesystemAdd(spNode pFile);
			virtual bool filesystemSave();
			virtual d64::spFile filesystemFindFreeIndex(d64::spFile pFile);

			virtual std::string imageType() const {
				return "Commodore 64 Disk Image (D64)";
			}

			virtual std::vector<std::string> imageExtensions() const {
				return { "d64" };
			}

			virtual tSector sectorCount(const tTrack pTrack = 0) const;
			virtual size_t sectorSize(const tTrack pTrack = 0) const;

			virtual bool sectorIsFree(const tTrackSector pTS) const;
			virtual bool sectorSet(const tTrackSector pTS, const bool pValue);

			virtual std::vector<tTrackSector> sectorsUse(const tSector pTotal);
			virtual bool sectorsFree(const std::vector<tTrackSector> pSectors);

			virtual std::vector<tTrackSector> sectorsGetFree(const tTrack pTrack = 0) const;

		protected:
			virtual bool filesystemChainLoad(spFile pFile);
			virtual d64::spFile filesystemEntryLoad(spBuffer pBuffer, const size_t pOffset);
			virtual bool filesystemEntrySave(d64::spFile pFile);

			virtual bool filesystemBitmapLoad();
			virtual bool filesystemBitmapSave();

		private:
			std::vector<d64::sTrackBam> mBam;

			uint8_t		mDosVersion;
			uint16_t	mDosType;
			std::string mLabel;
		};

	}
}
