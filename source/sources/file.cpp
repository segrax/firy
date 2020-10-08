#include "firy.hpp"
#include <filesystem>

namespace firy {

	namespace sources {

		/**
		 * Constructor
		 */
		cFile::cFile(const size_t pChunkSize) : cSource(pChunkSize) {

		}

		/**
		 * Open an image
		 */
		bool cFile::open(const std::string pFile) {
			mSourceFilename = pFile;
			mSourceSize = gResources->FileSize(pFile);
			return gResources->FileExists(pFile);
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
		spBuffer cFile::bufferChunk(const size_t pOffset) {
			size_t index = pOffset / mSourceChunkSize;
			size_t offset = pOffset % mSourceChunkSize;

			if (mBuffers.find(index) == mBuffers.end()) {
				auto chunk = gResources->FileRead(mSourceFilename, pOffset - offset, mSourceChunkSize);
				if (!chunk->size())
					return 0;

				mBuffers.insert({ index, chunk });
				return chunk;
			}

			auto buffer = mBuffers[index];
			return buffer;
		}

		/**
		 * Read a chunk of the buffer
		 */
		spBuffer cFile::bufferCopy(const size_t pOffset, const size_t pSize) {
			size_t remainSize = pSize;
			size_t mainoffset = pOffset;

			auto result = std::make_shared<tBuffer>();
			result->resize(pSize);

			auto ptr = result->data();
			while (remainSize && ptr < result->data() + result->size()) {
				auto Buffer = bufferChunk(mainoffset);
				if (!Buffer)
					return 0;

				size_t offset = mainoffset % mSourceChunkSize;
				size_t size = Buffer->size() - offset;

				size_t maxSize = (pSize < size) ? pSize : size;
				if (remainSize < maxSize)
					maxSize = remainSize;

				memcpy(ptr, Buffer->data() + offset, maxSize);
				remainSize -= maxSize;
				ptr += maxSize;

				mainoffset += size;
			}

			return result;
		}

		spSourceFile OpenFile(const std::string& pFile) {
			auto file = std::make_shared<firy::sources::cFile>();
			if (!file->open(pFile)) {
				return 0;
			}
			return file;
		}
	}
}
