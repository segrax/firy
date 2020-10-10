#include "firy.hpp"

namespace firy {
	namespace images {

		/**
		 * Constructor
		 */
		cImage::cImage() :
			access::cInterface(0),
			filesystem::cInterface(),
			std::enable_shared_from_this<cImage>() {

		};
	}
}
