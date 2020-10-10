#include "firy.hpp"
#include <filesystem>

namespace firy {

	namespace sources {

		/**
		 * Constructor
		 */
		cFile::cFile(const size_t pChunkSize) : cInterface(pChunkSize) {

		}

		/**
		 * Open an image
		 */
		bool cFile::open(const std::string pFile) {
			mFilename = pFile;
			mSourceSize = gResources->FileSize(pFile);
			return mSourceSize != 0;
		}

		/**
		 * Clear all buffers
		 */
		void cFile::close() {
			mBuffers.clear();
		}

		/**
		 * Get an image chunk, from memory or from disk
		 */
		spBuffer cFile::chunk(const size_t pOffset) {
			size_t index = pOffset / mSourceChunkSize;
			size_t offset = pOffset % mSourceChunkSize;

			if (mBuffers.find(index) == mBuffers.end()) {
				auto chunk = gResources->FileRead(mFilename, pOffset - offset, mSourceChunkSize);
				if (!chunk->size())
					return 0;

				mBuffers.insert({ index, chunk });
				return chunk;
			}

			auto buffer = mBuffers[index];
			return buffer;
		}

	}
}
