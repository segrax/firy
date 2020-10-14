#include "firy.hpp"
#include <filesystem>

namespace firy {

	namespace sources {

		/**
		 * Constructor
		 */
		cInterface::cInterface(const size_t pChunkSize) {
			mSourceSize = 0;
			chunkSizeSet(pChunkSize);
			mCreating = false;
		}

		/**
		 * Set the size of a chunk
		 */
		void cInterface::chunkSizeSet(const size_t pChunkSize) {
			mSourceChunkSize = pChunkSize;
			mBuffers.clear();
		}

		/**
		 * Create chunk(s) to set a minimum size of an image
		 *
		 * This would be used if you require a specific image size
		 *  But dont want to adjust mSourceChunkSize to be a multiple of it
		 */
		bool cInterface::chunkPrepare(size_t pSize) {
			size_t index = 0;

			while (pSize) {
				auto chunk = std::make_shared<tBuffer>();
				auto chunkSize = mSourceChunkSize;

				if (pSize < mSourceChunkSize) {
					chunkSize = pSize;
				}

				chunk->resize(chunkSize);
				mBuffers.insert({ index++, chunk });
				pSize -= chunkSize;
			}

			dirty();
			return true;
		}

		/**
		 * Copy into a new buffer
		 */
		spBuffer cInterface::chunkCopyToBuffer(const size_t pOffset, const size_t pSize) {
			auto result = std::make_shared<tBuffer>();
			result->resize(pSize);

			auto size = chunkCopyToPtr((uint8_t*)result->data(), pSize, pOffset);
			if (size != pSize)
				return 0;
			return result;
		}

		/**
		 * Copy from source into a buffer 
		 */
		size_t cInterface::chunkCopyToPtr(uint8_t* pTarget, const size_t pSize, const size_t pOffset) {
			size_t remainSize = pSize;
			size_t mainoffset = pOffset;

			auto ptr = pTarget;
			auto ptrEnd = ptr + pSize;

			while (remainSize && ptr < ptrEnd) {
				auto Buffer = chunk(mainoffset);
				if (!Buffer)
					return 0;

				size_t offset = mainoffset % mSourceChunkSize;
				size_t size = Buffer->size() - offset;

				// Read past end of buffer?
				if (Buffer->size() < offset) {
					return (pTarget - ptr);
				}

				size_t maxSize = (pSize < size) ? pSize : size;
				if (remainSize < maxSize)
					maxSize = remainSize;


				memcpy(ptr, Buffer->data() + offset, maxSize);
				remainSize -= maxSize;
				ptr += maxSize;

				mainoffset += size;
			}

			return pSize;
		}

		/**
		 * Write into the source chunk
		 */
		bool cInterface::chunkCopyFromBuffer(const size_t pOffset, spBuffer pBuffer) {
			size_t remainSize = pBuffer->size();
			size_t mainoffset = pOffset;

			size_t ptrOffset = 0;
			auto ptrEnd = pBuffer->size();

			while (remainSize && ptrOffset < ptrEnd) {
				auto Buffer = chunk(mainoffset);
				if (!Buffer)
					return 0;

				size_t offset = mainoffset % mSourceChunkSize;
				size_t size = Buffer->size() - offset;

				// Write past end of buffer?
				if (Buffer->size() < offset) {
					return false;
				}

				size_t maxSize = (remainSize < size) ? remainSize : size;
				Buffer->write(offset, pBuffer, ptrOffset, maxSize);
				remainSize -= maxSize;
				ptrOffset += maxSize;

				mainoffset += size;
			}
			
			dirty();
			pBuffer->dirty(false);
			return true;
		}

		/**
		 * Write into the source chunk
		 */
		bool cInterface::chunkCopyFromPtr(uint8_t* pSource, const size_t pSize, const size_t pOffset) {
			size_t remainSize = pSize;
			size_t mainoffset = pOffset;

			auto ptrOffset = pSource;
			auto ptrEnd = pSource + pSize;

			while (remainSize && ptrOffset < ptrEnd) {
				auto Buffer = chunk(mainoffset);
				if (!Buffer)
					return false;

				size_t offset = mainoffset % mSourceChunkSize;
				size_t size = Buffer->size() - offset;

				// Write past end of buffer?
				if (Buffer->size() < offset) {
					return false;
				}

				size_t maxSize = (remainSize < size) ? remainSize : size;
				Buffer->write(offset, ptrOffset, maxSize);
				remainSize -= maxSize;
				ptrOffset += maxSize;

				mainoffset += size;
			}

			dirty();
			return true;
		}

		/**
		 * Size in bytes
		 */
		size_t cInterface::size() const { 
			return mSourceSize; 
		};

		/**
		 * Get a pointer to a buffer chunk
		 */
		uint8_t* cInterface::sourceChunkPtr(const size_t pOffset) {
			spBuffer buffer = chunk(pOffset);

			return (buffer->data() + (pOffset % mSourceChunkSize));
		}

	}
}
