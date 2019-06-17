#include "firy.hpp"
#include <filesystem>

namespace firy {

	namespace images {

		cImage::cImage() {
			mBuffer = std::make_shared<tBuffer>();
		}

		bool cImage::imageOpen(const std::string pFile) {
			mBuffer = gResources->FileRead(pFile);
			return mBuffer != 0;
		}

		void cImage::imageClose() {
			mBuffer->clear();
		}
	}
}
