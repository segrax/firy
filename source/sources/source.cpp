#include "firy.hpp"
#include <filesystem>

namespace firy {

	namespace sources {

		/**
		 * Constructor
		 */
		cSource::cSource(const size_t pChunkSize) {
			mSourceSize = 0;
			mSourceChunkSize = pChunkSize;
		}

	}
}
