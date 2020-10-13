#include "firy.hpp"

#include "images/d64.hpp"
#include "images/adf.hpp"
#include "images/fat.hpp"

namespace firy {

	std::shared_ptr<cResources> gResources;

	std::shared_ptr<cFiry> gFiry = std::make_shared<cFiry>();
	std::shared_ptr<cDebug> gDebug = std::make_shared<cDebug>(0);

	/**
	 *
	 */
	cFiry::cFiry() {
		gResources = std::make_shared<cResources>();
	}

	/**
	 * Create a file as a source
	 */
	spSource cFiry::createLocalFile(const std::string& pFilename) {
		auto file = std::make_shared<firy::sources::cFile>();
		if (!file->create(pFilename)) {
			return 0;
		}
		return file;
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

		if (!image || image->filesystemLoad() == false && !pIgnoreValid) {
			return 0;
		}

		return image;
	}


	/**
	 *
	 */
	spImage cFiry::openImage(const std::string& pFilename) {
		gDebug->outputDisable(true);

		spImage file = 0;

		// D64
		try {
			file = openImageFile<images::cD64>(pFilename);
		} catch (std::exception exception) {
		}

		// ADF
		try {
			if (!file)
				file = openImageFile<images::cADF>(pFilename);
		} catch (std::exception exception) {
		}

		// FAT
		try {
			if (!file)
				file = openImageFile<images::cFAT>(pFilename);
		} catch (std::exception exception) {
		}

		gDebug->outputDisable(false);
		return file;
	}
}
