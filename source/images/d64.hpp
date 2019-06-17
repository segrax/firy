namespace firy {
	namespace images {

		enum eD64FileType {
			eD64FileType_DEL = 0,
			eD64FileType_SEQ = 1,
			eD64FileType_PRG = 2,
			eD64FileType_USR = 3,
			eD64FileType_REL = 4,
			eD64FileType_UNK
		};

		/**
		 * Commodore 64: Representation of a file on a disk
		 */
		struct sD64File : filesystem::sFile {
			size_t mSizeInSectors;
			eD64FileType mType;

			sD64File(wpFilesystem pFilesystem);
			spBuffer read();
		};

		typedef std::shared_ptr<sD64File> spD64File;

		/**
		 * Commodore 64: Disk Image (D64)
		 */
		class cD64 : public cDisk {

		public:

			cD64();

			virtual bool filesystemPrepare();
			spBuffer filesystemRead(spNode pFile);

			virtual spBuffer trackRead(const tTrackSector pTrack);
			virtual bool trackWrite(const tTrackSector pBlock, const spBuffer pBuffer);

			virtual tSector sectorCount(const tTrack pTrack = 0) const;
			virtual size_t sectorSize(const tTrack pTrack = 0) const;

		protected:
			virtual bool filesystemChainLoad(spFile pFile);

			virtual spD64File filesystemEntryProcess(const uint8_t* pBuffer);

		private:

			
		};


	}
}
