#include "firy.hpp"

namespace firy {

	std::shared_ptr<cResources> gResources;

	std::shared_ptr<cFiry> gFiry = std::make_shared<cFiry>();

	cFiry::cFiry() {
		gResources = std::make_shared<cResources>();
	}
}
