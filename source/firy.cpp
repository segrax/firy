#include "firy.hpp"

#include "images/d64.hpp"
#include "images/adf.hpp"
#include "images/fat.hpp"

namespace firy {

	std::shared_ptr<cResources> gResources;

	std::shared_ptr<cFiry> gFiry = std::make_shared<cFiry>();

	/**
	 *
	 */
	cFiry::cFiry() {
		gResources = std::make_shared<cResources>();
	}

	/**
	 * Open a file as a source
	 */
	spSource cFiry::openLocalFile(const std::string& pFilename) {
		auto file = std::make_shared<firy::sources::cFile>();
		if (!file->open(pFilename)) {
			return 0;
		}
		return file;
	}

	/**
	 * Open an image from a local file
	 */
	template <class tImageType> std::shared_ptr<tImageType> cFiry::openImageFile(const std::string& pFilename, const bool pIgnoreValid) {

		// Detect filetype
		auto image = std::make_shared<tImageType>(firy::gFiry->openLocalFile(pFilename));

		if (image->filesystemPrepare() == false && !pIgnoreValid) {
			return 0;
		}

		return image;
	}

	/**
	 *
	 */
	spImage cFiry::openImage(const std::string& pFilename) {

		spImage file = openImageFile<images::cD64>(pFilename);

		if(!file)
			file = openImageFile<images::cADF>(pFilename);

		if(!file)
			file = openImageFile<images::cFAT>(pFilename);

		return file;
	}
}
