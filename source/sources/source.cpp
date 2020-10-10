#include "firy.hpp"
#include <filesystem>

namespace firy {

	namespace sources {

		/**
		 * Constructor
		 */
		cInterface::cInterface(const size_t pChunkSize) {
			mSourceSize = 0;
			mSourceChunkSize = pChunkSize;
		}

		/**
		 * Copy into a new buffer
		 */
		spBuffer cInterface::bufferCopy(const size_t pOffset, const size_t pSize) {
			auto result = std::make_shared<tBuffer>();
			result->resize(pSize);

			auto size = bufferCopyTo((uint8_t*)result->data(), pSize, pOffset);
			if (size != pSize)
				return 0;
			return result;
		}

		/**
		 * Copy into a buffer 
		 */
		size_t cInterface::bufferCopyTo(uint8_t* pTarget, const size_t pSize, const size_t pOffset) {
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

		size_t cInterface::size() const { 
			return mSourceSize; 
		};

		/**
		 * Get a pointer to a buffer chunk
		 */
		uint8_t* cInterface::chunkPtr(const size_t pOffset) {
			spBuffer buffer = chunk(pOffset);

			return (buffer->data() + (pOffset % mSourceChunkSize));
		}

	}
}
