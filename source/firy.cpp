#include "firy.hpp"

#include "images/d64.hpp"
#include "images/adf.hpp"
#include "images/fat.hpp"

namespace firy {

	std::shared_ptr<cResources> gResources;

	std::shared_ptr<cFiry> gFiry = std::make_shared<cFiry>();
	std::shared_ptr<cConsole> gConsole = std::make_shared<cConsole>(0);
	std::shared_ptr<cOptions> gOptionsDefault = std::make_shared<cOptions>();

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
	template <class tImageType> std::shared_ptr<tImageType> cFiry::openImageFile(const std::string& pFilename, spOptions pOptions, const bool pIgnoreValid) {

		// Detect filetype
		auto image = std::make_shared<tImageType>(firy::gFiry->openLocalFile(pFilename));
		if (image)
			image->optionsSet(pOptions);

		if (!image || image->filesystemLoad() == false && !pIgnoreValid) {
			return 0;
		}

 		return image;
	}

	/**
	  * Get a list of known file extensions
	  */
	std::vector<std::string> cFiry::getKnownExtensions() {
		std::vector<std::string> extensions;

		for (auto& ext : images::cD64::imageExtensions()) {
			extensions.push_back(ext);
		}
		for (auto& ext : images::cADF::imageExtensions()) {
			extensions.push_back(ext);
		}
		for (auto& ext : images::cFAT::imageExtensions()) {
			extensions.push_back(ext);
		}
		return extensions;
	}

	/**
	 *
	 */
	spImage cFiry::openImage(const std::string& pFilename) {
		spImage file = 0;

		spOptions options = gOptionsDefault->clone();
		options->errorShowSet(false);

		// D64
		try {
			file = openImageFile<images::cD64>(pFilename, options);
		} catch (std::exception exception) {
		}

		// ADF
		try {
			if (!file)
				file = openImageFile<images::cADF>(pFilename, options);
		} catch (std::exception exception) {
		}

		// FAT
		try {
			if (!file)
				file = openImageFile<images::cFAT>(pFilename, options);
		} catch (std::exception exception) {
		}

		return file;
	}
}
