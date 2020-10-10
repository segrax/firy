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

			/**
			 * Commodore 64: Representation of a file on a disk
			 */
			struct sFile : filesystem::sFile {
				size_t mSizeInSectors;
				eFileType mType;

				sFile(wpFilesystem pFilesystem);
			};

			typedef std::shared_ptr<sFile> spFile;

			/**
			 * Commodore 64: Track Bam entry
			 */
			struct sTrackBam {
				uint8_t mFreeSectors;
				uint8_t m0;	// Sector 00 to 07
				uint8_t m1; // Sector 08 to 15
				uint8_t m2; // Sector 16 to 23

			};
		};

		/**
			* Commodore 64: Disk Image (D64)
			*/
		class cD64 : public cImageAccess<access::cTracks> {

		public:
			cD64(spSource pSource);

			virtual std::string filesystemNameGet();
			virtual bool filesystemPrepare();
			virtual spBuffer filesystemRead(spNode pFile);

			virtual std::string imageType() const {
				return "Commodore 64 Disk";
			}

			virtual std::vector<std::string> imageExtensions() const {
				return { "d64" };
			}

			virtual tSector sectorCount(const tTrack pTrack = 0) const;
			virtual size_t sectorSize(const tTrack pTrack = 0) const;

		protected:
			virtual bool filesystemChainLoad(spFile pFile);
			virtual d64::spFile filesystemEntryProcess(spBuffer pBuffer, size_t pOffset);
			virtual bool filesystemBitmapLoad();

		private:
			std::vector<d64::sTrackBam> mBam;

			uint8_t		mDosVersion;
			uint16_t	mDosType;
			std::string mLabel;
		};

	}
}
