#include "firy.hpp"
#include <filesystem>

namespace firy {

	namespace images {

		cImage::cImage(const size_t pChunkSize) {
			mImageSize = 0;
			mImageChunkSize = pChunkSize;
		}

		bool cImage::imageOpen(const std::string pFile) {
			mImageFilename = pFile;
			mImageSize = gResources->FileSize(pFile);
			return gResources->FileExists(pFile);
		}

		void cImage::imageClose() {
			mBuffers.clear();
		}

		/**
		 * Get an image chunk, from memory or from disk
		 */
		spBuffer cImage::imageChunkBuffer(const size_t pOffset) {
			size_t index = pOffset / mImageChunkSize;
			size_t offset = pOffset % mImageChunkSize;

			if (mBuffers.find(index) == mBuffers.end()) {
				auto chunk = gResources->FileRead(mImageFilename, pOffset - offset, mImageChunkSize);
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
		std::shared_ptr<tBuffer> cImage::imageBufferCopy(const size_t pOffset, const size_t pSize) {
			size_t remainSize = pSize;
			size_t mainoffset = pOffset;

			auto result = std::make_shared<tBuffer>();
			result->resize(pSize);

			auto ptr = result->data();
			while (remainSize && ptr < result->data() + result->size()) {
				auto Buffer = imageChunkBuffer(mainoffset);
				if (!Buffer)
					return 0;

				size_t offset = mainoffset % mImageChunkSize;
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

	}
}
