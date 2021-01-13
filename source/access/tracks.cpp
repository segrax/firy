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

#include "firy.hpp"

namespace firy {
	namespace access {

		/**
		 * Constructor
		 */
		cTracks::cTracks() {
			mTrackCount = 0;
		}

		/**
		 * Number of tracks
		 */
		inline tTrack cTracks::trackCount() const {
			return mTrackCount;
		}

		/**
		 * Number of bytes per track
		 */
		size_t cTracks::trackSize(const tTrack pTrack) const {
			return sectorCount(pTrack) * sectorSize(pTrack);
		}

		/**
		 * Get the offset from the start of the image, to the track
		 */
		size_t cTracks::trackOffset(const tTrack pTrack) const {
			size_t offset = 0;

			// Invalid track?
			if (!trackValid(pTrack))
				return 0;

			// Loop through each track, and add up
			for (tTrack track(1);
				 track < pTrack && track <= trackCount();
				 track++) {

				offset += trackSize(track);
			}

			return offset;
		}
		
		/**
		 * Read a track
		 */
		spBuffer cTracks::trackRead(const tTrack pTrack) {
			return sourceBufferCopy(trackOffset(pTrack), trackSize(pTrack));
		}

		/**
		 * Write a track
		 */
		bool cTracks::trackWrite(const tTrack pTrack, const spBuffer pBuffer) {
			// TODO
			return false;
		}
		/**
		 * Read a sector
		 */
		spBuffer cTracks::sectorRead(const tTrackSector pTS) {
			if (!sectorValid(pTS))
				return 0;

			return sourceBufferCopy(sectorOffset(pTS), sectorSize(pTS.first));
		}

		/**
		 * Write a sector
		 */
		bool cTracks::sectorWrite(const tTrackSector pTS, const spBuffer pBuffer) {
			
			return sourceBufferWrite(sectorOffset(pTS), pBuffer);
		}

		/**
		 * Get the offset from the start of the image, to the track/sector
		 */
		size_t cTracks::sectorOffset(const tTrackSector pTS) const {
			return trackOffset(pTS.first) + (sectorSize() * pTS.second);
		}
	}
}
