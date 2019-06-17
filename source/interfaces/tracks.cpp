#include "firy.hpp"

namespace firy {

	typedef size_t tTrack;
	typedef size_t tSector;

	typedef std::pair<tTrack, tSector> tTrackSector;

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
		size_t cTracks::trackOffset(const tTrackSector pTrack) const {
			size_t offset = 0;

			// Invalid track?
			if (pTrack.first <= 0 || pTrack.first > trackCount())
				return 0;

			// Loop through each track, and add up
			for (tTrackSector ts(1, 0);
				 ts.first < pTrack.first && ts.first <= trackCount();
				 ts.first++) {

				offset += trackSize(ts.first);
			}

			return offset;
		}

		/**
		 * Get the offset from the start of the image, to the track/sector
		 */
		size_t cTracks::sectorOffset(const tTrackSector pTS) const {
			return trackOffset(pTS) + (sectorSize() * pTS.second);
		}
	}
}
