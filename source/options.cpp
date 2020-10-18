#include "firy.hpp"

namespace firy {


	cOptions::cOptions() {
		mErrorShow = true;
		mWarningShow = true;
		mAutoSaveSource = false;
		mAutoSaveSourceExit = false;
	}

	/**
	 * Display a warning
	 */
	spOptionResponse cOptions::warning(pImage pImage, const std::string& pMessage, const std::string& pMessageDetail) {
		if (!mWarningShow)
			return std::make_shared<firy::cOptionResponse>(true);

		gConsole->warning(pImage->sourceID(), pMessage, pMessageDetail);
		return std::make_shared<firy::cOptionResponse>(false);
	}

	/**
	 * Display an error
	 */
	void cOptions::error(pImage pImage, const std::string& pMessage, const std::string& pMessageDetail) {
		if (!mErrorShow)
			return;

		gConsole->error(pImage->sourceID(), pMessage, pMessageDetail);
	}
	
	/**
	 *
	 */
	spOptionResponse cOptions::savechanges(pImage pImage, const std::string& pMessage) {
		if (mAutoSaveSource)
			return std::make_shared<firy::cOptionResponse>(false);

		return std::make_shared<firy::cOptionResponse>(true);
	}

	/**
	 *
	 */
	spOptionResponse cOptions::savechangesExit(pImage pImage, const std::string& pMessage) {
		if (mAutoSaveSourceExit)
			return std::make_shared<firy::cOptionResponse>(false);

		gConsole->warning(pImage->sourceID(), pMessage);
		return std::make_shared<firy::cOptionResponse>(true);
	}

	/**
	 * Enable error display
	 */
	void cOptions::errorShowSet(const bool pEnabled) {
		mErrorShow = pEnabled;
	}

	/**
	 * Enable warning display
	 */
	void cOptions::warningShowSet(const bool pEnabled) {
		mErrorShow = pEnabled;
	}
}
