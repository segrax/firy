#include "firy.hpp"

namespace firy {
	namespace interfaces {

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
		 * Get the offset from the start of the image, to the track/sector
		 */
		size_t cTracks::sectorOffset(const tTrackSector pTS) const {
			return trackOffset(pTS.first) + (sectorSize() * pTS.second);
		}
	}
}
