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

namespace firy {

	/**
	 * Take bytes off the start of the vector
	 */
	spBuffer cBuffer::takeBytes(const size_t pBytes) {
		tLockGuard lock(mLock);

		if (!pBytes || pBytes > size()) {
			return 0;
		}

		spBuffer buffer = std::make_shared<tBuffer>();
		buffer->resize(pBytes);

		dirty(true);
		memcpy(buffer->data(), data(), pBytes);
		erase(begin(), begin() + pBytes);
		return buffer;
	}

	/**
	 * Read a string from the buffer
	 */
	std::string cBuffer::getString(const size_t pOffset, const size_t pLengthMax, const uint8_t pTerminator) {
		tLockGuard lock(mLock);
		std::string tmpString;
		const uint8_t* buffer = reinterpret_cast<const uint8_t*>(&at(pOffset));

		for (size_t i = 0; *buffer != pTerminator && i <= pLengthMax; ++i)
			tmpString += (char)* buffer++;

		return tmpString;
	}

	/**
	 * Ensure an offset and size is valid
	 */
	inline void cBuffer::assertOffset(const size_t pOffset, const size_t pBytes) const {
		if ((pOffset + pBytes) > size())
			throw std::exception("Read past end of buffer");
	}

	/**
	 * Resize the buffer
	 *
	 * LOCK BEFORE CALLING
	 */
	inline void cBuffer::expandOffset(const size_t pOffset, const size_t pBytes) {
		size_t newSize = pOffset + pBytes;
		if (newSize <= size())
			return;

		resize(newSize);
	}

	/**
	 * Get a byte
	 */
	uint8_t cBuffer::getByte(size_t pOffset) const {
		assertOffset(pOffset);
		return at(pOffset);
	}

	/** 
	 * TODO: Need endian switch around these functions for platform
	 */


	/**
	 * Get a word, little-endian
	 */
	uint16_t cBuffer::getWordLE(const size_t pOffset) const {
		assertOffset(pOffset, 2);
		const uint16_t* bytes = reinterpret_cast<const uint16_t*>(&at(pOffset));
		return *bytes;
	}

	/**
	 * Get a double-word, little-endian
	 */
	uint32_t cBuffer::getDWordLE(const size_t pOffset) const {
		assertOffset(pOffset, 4);
		const uint32_t* bytes = reinterpret_cast<const uint32_t*>(&at(pOffset));
		return *bytes;
	}

	/**
	 * Get a word, big-endian
	 */
	uint16_t cBuffer::getWordBE(const size_t pOffset) const {
		assertOffset(pOffset, 2);
		const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&at(pOffset));
		return uint16_t((bytes[0] << 8) + bytes[1]);
	}

	/**
	 * Get a double-word, big-endian
	 */
	uint32_t cBuffer::getDWordBE(const size_t pOffset) const {
		assertOffset(pOffset, 4);
		const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&at(pOffset));
		return uint32_t((bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + (bytes[3]));
	}

	/**
	 *
	 */
	void cBuffer::pushByte(const uint8_t pByte) {
		tLockGuard lock(mLock);
		dirty(true);
		push_back(pByte);
	}

	/**
	 *
	 */
	void cBuffer::pushWord(const uint16_t pWord) {
		tLockGuard lock(mLock);
		dirty(true);
		const uint8_t* bytes = (const uint8_t*)& pWord;

		push_back(bytes[0]);
		push_back(bytes[1]);
	}

	void cBuffer::pushDWord(const uint32_t pDWord) {
		tLockGuard lock(mLock);
		dirty(true);
		const uint8_t* bytes = (const uint8_t*)& pDWord;

		push_back(bytes[0]);
		push_back(bytes[1]);
		push_back(bytes[2]);
		push_back(bytes[3]);
	}

	/**
	 * Push an entire buffer
	 */
	void cBuffer::pushBuffer(std::shared_ptr<cBuffer> pBuffer) {
		tLockGuard lock(mLock);
		tLockGuard lock2(pBuffer->mLock);
		dirty(true);
		insert(end(), pBuffer->begin(), pBuffer->end());
	}

	/**
	 * Push a buffer, starting at an offset in the source, for a max number of bytes
	 */
	bool cBuffer::pushBuffer(std::shared_ptr<cBuffer> pBuffer, const size_t pSourceOffset, const size_t pMax) {
		tLockGuard lock(mLock);
		tLockGuard lock2(pBuffer->mLock);
		dirty(true);
		auto beginIT = pBuffer->begin() + pSourceOffset;

		if (beginIT + pMax > pBuffer->end())
			return false;

		insert(end(), beginIT, beginIT + pMax);
		return true;
	}

	/**
	 * Push a buffer, starting at the beginning of source, for a max number of bytes
	 */
	void cBuffer::pushBuffer(std::shared_ptr<cBuffer> pBuffer, const size_t pMax) {
		tLockGuard lock(mLock);
		tLockGuard lock2(pBuffer->mLock);
		if (pMax > pBuffer->size()) {
			return;
		}
		dirty(true);
		insert(end(), pBuffer->begin(), pBuffer->begin() + pMax);
	}

	void cBuffer::pushBuffer(const uint8_t* pBuffer, const size_t pSize) {
		tLockGuard lock(mLock);
		dirty(true);
		insert(end(), pBuffer, pBuffer + pSize);
	}

	void cBuffer::putByte(const size_t pOffset, uint8_t pByte) {
		tLockGuard lock(mLock);
		expandOffset(pOffset);
		dirty(true);
		at(pOffset) = pByte;
	}

	void cBuffer::putWordLE(const size_t pOffset, uint16_t pByte) {
		tLockGuard lock(mLock);
		expandOffset(pOffset, 2);
		dirty(true);
		uint16_t* bytes = reinterpret_cast<uint16_t*>(&at(pOffset));
		*bytes = pByte;
	}

	void cBuffer::putDWordLE(const size_t pOffset, uint32_t pByte) {
		tLockGuard lock(mLock);
		expandOffset(pOffset, 4);
		dirty(true);
		uint32_t* bytes = reinterpret_cast<uint32_t*>(&at(pOffset));
		*bytes = pByte;
	}

	void cBuffer::putWordBE(const size_t pOffset, uint16_t pByte) {
		tLockGuard lock(mLock);
		expandOffset(pOffset, 2);
		dirty(true);
		uint8_t* bytes = reinterpret_cast<uint8_t*>(&at(pOffset));
		bytes[0] = pByte >> 8;
		bytes[1] = pByte & 0xFF;
	}

	void cBuffer::putDWordBE(const size_t pOffset, uint32_t pByte) {
		tLockGuard lock(mLock);
		expandOffset(pOffset, 4);
		dirty(true);
		uint8_t* bytes = reinterpret_cast<uint8_t*>(&at(pOffset));
		bytes[0] = pByte >> 24;
		bytes[1] = pByte >> 16;
		bytes[2] = pByte >> 8;
		bytes[3] = pByte & 0xFF;
	}

	void cBuffer::putBuffer(const size_t pOffset, std::shared_ptr<cBuffer> pBuffer) {
		tLockGuard lock(mLock);
		expandOffset(pOffset, pBuffer->size());
		dirty(true);
		auto buf = data() + pOffset;
		memcpy(buf, pBuffer->data(), pBuffer->size());
	}

	void cBuffer::putString(const size_t pOffset, const std::string& pBuffer) {
		tLockGuard lock(mLock);
		expandOffset(pOffset, pBuffer.size());
		dirty(true);
		auto buf = data() + pOffset;
		memcpy(buf, pBuffer.data(), pBuffer.size());
	}

	/**
	 * Compare two buffers
	 */
	bool cBuffer::operator==(const cBuffer& pBuffer) {
		tLockGuard lock(mLock);
		tLockGuard lock2(pBuffer.mLock);
		if (size() != pBuffer.size())
			return false;
		return std::equal(begin(), end(), pBuffer.begin());
	}

	bool cBuffer::operator!=(const cBuffer& pBuffer) {
		return !this->operator==(pBuffer);
	}

	/**
	 * Write to a buffer
	 */
	bool cBuffer::write(const size_t pOffset, const spBuffer pBuffer) {
		tLockGuard lock(mLock);
		tLockGuard lock2(pBuffer->mLock);
		size_t maxSize = size() - pOffset;
		if (maxSize < pBuffer->size())
			return false;

		dirty(true);
		memcpy(&at(pOffset), pBuffer->data(), pBuffer->size());
		return true;
	}

	/**
	 *
	 */
	bool cBuffer::write(const size_t pOffset, const spBuffer pBuffer, const size_t pOffsetStart, const size_t pSize) {
		tLockGuard lock(mLock); 
		tLockGuard lock2(pBuffer->mLock);
		size_t maxSize = size() - pOffset;
		if (maxSize < pBuffer->size())
			return false;

		if (pBuffer->size() - pOffsetStart < pSize) {
			return false;
		}

		memcpy(&at(pOffset), pBuffer->data() + pOffsetStart, pSize);
		dirty(true);
		return true;
	}

	/**
	 *
	 */
	bool cBuffer::write(const size_t pOffset, const uint8_t* pBuffer, const size_t pSize) {
		tLockGuard lock(mLock); 
		size_t maxSize = size() - pOffset;
		if (maxSize < pSize)
			return false;

		memcpy(&at(pOffset), pBuffer, pSize);
		dirty(true);
		return true;
	}

}
