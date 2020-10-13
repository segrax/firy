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
		 * Create a file
		 */
		bool cFile::create(const std::string pFile) {
			mCreating = true;
			return !open(pFile);
		}

		/**
		 * Open a file
		 */
		bool cFile::open(const std::string pFile) {
			mFilename = pFile;
			mSourceSize = gResources->FileSize(pFile);
			return mSourceSize != 0;
		}

		/**
		 * Save modified buffers to file
		 */
		bool cFile::save(const std::string pFile) {

			// Total new file?
			if (pFile.size() && pFile != mFilename) {
				gDebug->error("Save as not implemented");
				return false;
			}

			for (auto& buffer : mBuffers) {
				if (buffer.second->isDirty()) {
					size_t offset = buffer.first * mSourceChunkSize;
					if (!gResources->FileWrite(mFilename, offset, buffer.second)) {

						if (mCreating) {
							if (gResources->FileSave(mFilename, buffer.second))
								continue;
						}
						gDebug->error("Failed to save: ", mFilename);
						return false;
					}
				}
			}
			dirty(false);
			return true;

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
				if (!chunk->size()) {
					if (!mCreating)
						return 0;

					chunk = std::make_shared<tBuffer>();
					chunk->resize(mSourceChunkSize);
				}

				mBuffers.insert({ index, chunk });
				return chunk;
			}

			auto buffer = mBuffers[index];
			return buffer;
		}

	}
}
