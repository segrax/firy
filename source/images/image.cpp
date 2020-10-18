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

			optionsSet(gOptionsDefault);
		};

		/**
		 *
		 */
		cImage::~cImage() {

			if (sourceIsDirty() && !savechangesExit("Save Changes")->isAborted()) {
				sourceSave();
			}
		}

		/**
		 * Save the filesystem, and trigger a source save
		 */
		bool cImage::filesystemSave() {
			auto res = filesystemSaveNative();
			if (!res)
				return false;

			if (sourceIsDirty() && !savechanges("Save Changes")->isAborted()) {
				sourceSave();
			}
			return true;
		}

		/**
		 * Options
		 */
		void cImage::optionsSet(spOptions pOptions) {
			mOptions = pOptions->clone();
		}

	}
}
