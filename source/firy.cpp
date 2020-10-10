#include "firy.hpp"

namespace firy {

	std::shared_ptr<cResources> gResources;

	std::shared_ptr<cFiry> gFiry = std::make_shared<cFiry>();

	cFiry::cFiry() {
		gResources = std::make_shared<cResources>();
	}

	spSource cFiry::openFile(const std::string pFilename) {
		auto file = std::make_shared<firy::sources::cFile>();
		if (!file->open(pFilename)) {
			return 0;
		}
		return file;
	}


}
