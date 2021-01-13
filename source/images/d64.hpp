/*
 *  FIRY
 *  ---------------
 *
 *  Copyright (C) 2019-2021
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

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

			virtual std::string imageTypeShort() const override { return "d64"; }
			virtual std::string imageType() const override {
				return "Commodore 64 Disk Image (D64)";
			}

			static std::vector<std::string> imageExtensions() {
				return { "d64" };
			}

			virtual bool filesystemCreate() override;
			virtual bool filesystemLoad() override;
			virtual spBuffer filesystemRead(spNode pFile) override;
			virtual bool filesystemRemove(spNode pFile) override;

			spFile filesystemFileCreate(const std::string& pName = "") {
				return filesystemNodeCreate<d64::sFile>(pName);
			}

			spDirectory filesystemDirectoryCreate(const std::string& pName = "") {
				return 0;
			}

			d64::spFile filesystemFindFreeIndex(d64::spFile pFile);

			virtual size_t filesystemTotalBytesFree() override;
			virtual size_t filesystemTotalBytesMax() override;

			virtual tSector sectorCount(const tTrack pTrack = 0) const override;
			virtual size_t sectorSize(const tTrack pTrack = 0) const override;

			virtual bool sectorIsFree(const tTrackSector pTS) const override;
			virtual bool sectorSet(const tTrackSector pTS, const bool pValue) override;

			virtual std::vector<sAccessUnit> sectorsUse(const tSector pTotal) override;
			virtual bool sectorsFree(const std::vector<sAccessUnit> & pSectors) override;

			virtual std::vector<sAccessUnit> sectorsGetFree(const tTrack pTrack = 0) const override;

		protected:
			virtual bool filesystemChainLoad(spFile pFile);
			virtual d64::spFile filesystemEntryLoad(spBuffer pBuffer, const size_t pOffset);
			virtual bool filesystemEntrySave(d64::spFile pFile);

			virtual bool filesystemBitmapLoad() override;
			virtual bool filesystemBitmapSave() override;

			virtual bool filesystemSaveNative() override;
		private:
			std::vector<d64::sTrackBam> mBam;

			uint8_t		mDosVersion;
			uint16_t	mDosType;
			uint16_t	mDiskID;
		};

	}
}
