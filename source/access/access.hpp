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

namespace firy {

	namespace access {

		/**
		 * Provide function helpers to an underlying source
		 */
		class cInterface {

		public:
			/**
			 * Source-Access Interface
			 *
			 * Throws if shared_ptr is empty
			 */
			cInterface(spSource pSource) {
				mSource = pSource;

				assertSource();
			}

			/**
			 * Save changes back to source
			 */
			bool sourceSave(const std::string pID = "") {
				if (!mSource->save(pID))
					return false;
				return true;
			}

			/**
			 * Get a source id
			 */
			std::string sourceID() const {
				return mSource->sourceID();
			}

			virtual std::vector<sAccessUnit> unitGetFree() const = 0;
			virtual spBuffer unitRead(sAccessUnit pChain) = 0;

		protected:

			/**
			 * Throw an exception if a source wasn't provided
			 */
			inline void assertSource() const {
				if (!mSource)
					throw std::exception("Source was not found");
			}

			/**
			 * Get the size of the source in bytes
			 */
			size_t sourceSize() const {
				return mSource->size();
			}

			/**
			 * Is the source dirty
			 */
			bool sourceIsDirty() const {
				return mSource->hasDirtyBuffers();
			}

			/**
			 * Load a specific object from an offset in the source
			 */
			template <class tBlockType> std::shared_ptr<tBlockType> sourceObjectGet(const size_t pOffset = 0) {
				return mSource->objectGet<tBlockType>(pOffset);
			}

			/**
			 * Save a specific object to an offset in the source
			 */
			template <class tBlockType> bool sourceObjectPut(const size_t pOffset, std::shared_ptr<tBlockType> pObject) {
				return mSource->objectPut<tBlockType>(pOffset, pObject);
			}

			/**
			 * Return a copy of the source
			 */
			spBuffer sourceBufferCopy(const size_t pOffset = 0, const size_t pSize = 0) {
				return mSource->chunkCopyToBuffer(pOffset, pSize);
			}

			/**
			 * Get a chunk from the source (based on mSourceChunkSize)
			 */
			spBuffer sourceBufferChunk(const size_t pOffset = 0) {
				return mSource->chunk(pOffset);
			}

			/**
			 * Write to a chunk
			 */
			bool sourceBufferWrite(const size_t pOffset, const spBuffer pBuffer) {

				if (!mSource->chunkCopyFromBuffer(pOffset, pBuffer))
					return false;
				return true;
			}

			/**
			 * Prepare chunks upto a specific size
			 *
			 * This would be used if you require a specific image size
			 *  But dont want to adjust mSourceChunkSize to be a multiple of it
			 */
			bool sourceChunkPrepare(const size_t pSize) {
				if (!mSource->chunkPrepare(pSize))
					return false;
				return true;
			}

		protected:
			spSource mSource;

		};
	}
}
