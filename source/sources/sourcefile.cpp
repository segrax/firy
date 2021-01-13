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
			mSourceID = pFile;
			mSourceSize = gResources->FileSize(pFile);
			return mSourceSize != 0;
		}

		/**
		 * Save modified buffers to file
		 */
		bool cFile::save(const std::string pFile) {

			// Total new file?
			if (pFile.size() && pFile != mSourceID) {

				// TODO: We need to copy the source not in mBuffers

				for (auto& buffer : mBuffers) {
					size_t offset = buffer.first * mSourceChunkSize;
					if (!gResources->FileWrite(pFile, offset, buffer.second)) {
						if (gResources->FileSave(pFile, buffer.second))
							continue;
					}
				}

				return true;
			}

			for (auto& buffer : mBuffers) {
				if (buffer.second->isDirty()) {
					size_t offset = buffer.first * mSourceChunkSize;
					if (!gResources->FileWrite(mSourceID, offset, buffer.second)) {

						if (mCreating) {
							if (gResources->FileSave(mSourceID, buffer.second))
								continue;
						}
						gConsole->error("file", "Failed to save: ", mSourceID);
						return false;
					}
				}
			}
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

				auto chunk = gResources->FileRead(mSourceID, pOffset - offset, mSourceChunkSize);
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
