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

	namespace access {

		/**
		 * Provide a track/sector read/write interface
		 */
		class cTracks : public virtual access::cInterface {
		public:

			/**
			 * Constructor
			 */
			cTracks();

			/**
			 * Read the provided track
			 */
			virtual spBuffer trackRead(const tTrack pTrack);

			/**
			 * Write 'pBuffer' at provided track
			 */
			virtual bool trackWrite(const tTrack pTrack, const spBuffer pBuffer);

			/**
			 * Number of tracks
			 */
			virtual tTrack trackCount() const;

			/**
			 * Number of bytes per track
			 */
			virtual size_t trackSize(const tTrack pTrack = 0) const;
			
			/**
			 * Is a T/S free
			 */
			virtual bool sectorIsFree(const tTrackSector pTS) const = 0;

			/**
			 * Set a T/S used
			 */
			virtual bool sectorSet(const tTrackSector pTS, const bool pValue) = 0;

			/**
			 * Get 'pTotal' number of free sectors, marking them used
			 */
			virtual std::vector<sAccessUnit> sectorsUse(const tSector pTotal) = 0;

			/**
			 * Free all sectors in 'pSectors'
			 */
			virtual bool sectorsFree(const std::vector<sAccessUnit>& pSectors) = 0;

			/**
			 * Get the free sectors on a track
			 */
			virtual std::vector<sAccessUnit> sectorsGetFree(const tTrack pTrack = 0) const = 0;

			/**
			 * Read the provided T/S
			 */
			virtual spBuffer sectorRead(const tTrackSector pTS);

			/**
			 * Write 'pBuffer' at provided T/S
			 */
			virtual bool sectorWrite(const tTrackSector pTS, const spBuffer pBuffer);

			/**
			 * Number of sectors per track
			 */
			virtual tSector sectorCount(const tTrack pTrack = 0) const = 0;

			/**
			 * Number of bytes per sector
			 */
			virtual size_t sectorSize(const tTrack pTrack = 0) const = 0;

			/**
			 * Is this a valid track
			 */
			virtual bool trackValid(const tTrack pTrack) const {
				if (pTrack <= 0 || pTrack > trackCount())
					return false;
				return true;
			}

			/**
			 * Is this a valid TS
			 */
			virtual bool sectorValid(const tTrackSector pTS) const {
				if (!trackValid(pTS.first))
					return false;
				if (pTS.second >= sectorCount(pTS.first))
					return false;

				return true;
			}

			/**
			 * Get free sectors
			 */
			virtual std::vector<sAccessUnit> unitGetFree() const override {
				return sectorsGetFree();
			}

			/**
			 *
			 */
			virtual spBuffer unitRead(sAccessUnit pChain) override {

				return sectorRead(pChain.mTS);
			}
		protected:

			/**
			 * Get the offset from the start of the image, to the track
			 */
			virtual size_t trackOffset(const tTrack pTrack) const;
			
			/**
			 * Get the offset from the start of the image, to the track/sector
			 */
			virtual size_t sectorOffset(const tTrackSector pTS) const;

			/**
			 * Number of tracks
			 */
			tTrack mTrackCount;
		};
	}
}
