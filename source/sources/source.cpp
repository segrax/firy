/*
 *  FIRY
 *  ---------------
 *
 *  Copyright (C) 2019-2021
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

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
			std::lock_guard<std::recursive_mutex> lock(mAccess);
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
			std::lock_guard<std::recursive_mutex> lock(mAccess);
			size_t index = 0;
			mBuffers.clear();
			mSourceSize = pSize;

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

			return true;
		}

		/**
		 * Copy into a new buffer
		 */
		spBuffer cInterface::chunkCopyToBuffer(const size_t pOffset, const size_t pSize) {
			std::lock_guard<std::recursive_mutex> lock(mAccess);
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
			std::lock_guard<std::recursive_mutex> lock(mAccess);
			size_t remainSize = pSize;
			size_t mainoffset = pOffset;

			auto ptr = pTarget;
			auto ptrEnd = ptr + pSize;
			size_t bytesRead = 0;

			while (remainSize && ptr < ptrEnd) {
				auto Buffer = chunk(mainoffset);
				if (!Buffer)
					return bytesRead;

				size_t offset = mainoffset % mSourceChunkSize;
				size_t size = Buffer->size() - offset;

				// Read past end of buffer?
				if (Buffer->size() <= offset) {
					return bytesRead;
				}

				size_t maxSize = (remainSize < size) ? remainSize : size;
				size_t bytesRemain = ptrEnd - ptr;
				if (bytesRemain < maxSize)
					maxSize = bytesRemain;

				memcpy(ptr, Buffer->data() + offset, maxSize);
				remainSize -= maxSize;
				ptr += maxSize;
				bytesRead += maxSize;

				mainoffset += maxSize;
			}

			return bytesRead;
		}

		/**
		 * Write into the source chunk
		 */
		bool cInterface::chunkCopyFromBuffer(const size_t pOffset, spBuffer pBuffer) {
			std::lock_guard<std::recursive_mutex> lock(mAccess);
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
				if (Buffer->size() <= offset) {
					return false;
				}

				size_t maxSize = (remainSize < size) ? remainSize : size;
				Buffer->write(offset, pBuffer, ptrOffset, maxSize);
				remainSize -= maxSize;
				ptrOffset += maxSize;

				mainoffset += maxSize;
			}
			
			pBuffer->dirty(false);
			return remainSize == 0;
		}

		/**
		 * Write into the source chunk
		 */
		bool cInterface::chunkCopyFromPtr(uint8_t* pSource, const size_t pSize, const size_t pOffset) {
			std::lock_guard<std::recursive_mutex> lock(mAccess);
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
				if (Buffer->size() <= offset) {
					return false;
				}

				size_t maxSize = (remainSize < size) ? remainSize : size;
				Buffer->write(offset, ptrOffset, maxSize);
				remainSize -= maxSize;
				ptrOffset += maxSize;

				mainoffset += maxSize;
			}

			return remainSize == 0;
		}

		/**
		 * Size in bytes
		 */
		size_t cInterface::size() const { 
			std::lock_guard<std::recursive_mutex> lock(mAccess);
			return mSourceSize; 
		};

		/**
		 * Do we hold dirty buffers
		 */
		bool cInterface::hasDirtyBuffers() const {
			std::lock_guard<std::recursive_mutex> lock(mAccess);

			for (auto& buffer : mBuffers) {
				if (buffer.second->isDirty())
					return true;
			}
			return false;
		}

	}
}
