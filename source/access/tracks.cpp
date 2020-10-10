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
			if (pTrack <= 0 || pTrack > trackCount())
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
			return sourceBufferCopy(sectorOffset(pTS), sectorSize(pTS.first));
		}

		/**
		 * Write a sector
		 */
		bool cTracks::sectorWrite(const tTrackSector pTS, const spBuffer pBuffer) {
			// TODO
			return false;
		}

		/**
		 * Get the offset from the start of the image, to the track/sector
		 */
		size_t cTracks::sectorOffset(const tTrackSector pTS) const {
			return trackOffset(pTS.first) + (sectorSize() * pTS.second);
		}
	}
}
