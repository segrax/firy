#include <vector>
#include <memory>
#include <string>
#include "helpers/dirty.hpp"
#include "buffer.hpp"

namespace firy {

	/**
	 * Take bytes off the start of the vector
	 */
	spBuffer cBuffer::takeBytes(const size_t pBytes) {
		spBuffer buffer = std::make_shared<tBuffer>();

		buffer->resize(pBytes);
		if (pBytes > size()) {
			return 0;
		}
		dirty(true);
		memcpy(buffer->data(), data(), pBytes);
		erase(begin(), begin() + pBytes);
		return buffer;
	}

	std::string cBuffer::getString(const size_t pOffset, const size_t pLengthMax, const uint8_t pTerminator) {
		std::string tmpString;
		const uint8_t* buffer = reinterpret_cast<const uint8_t*>(&at(pOffset));

		for (size_t i = 0; *buffer != pTerminator && i <= pLengthMax; ++i)
			tmpString += (char)* buffer++;

		return tmpString;
	}

	inline void cBuffer::assertOffset(const size_t pOffset, const size_t pBytes) const {
		if ((pOffset + pBytes) > size())
			throw std::exception("Read past end of buffer");
	}

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

	void cBuffer::pushByte(const uint8_t pByte) {
		dirty(true);
		push_back(pByte);
	}

	void cBuffer::pushWord(const uint16_t pWord) {
		dirty(true);
		const uint8_t* bytes = (const uint8_t*)& pWord;

		push_back(bytes[0]);
		push_back(bytes[1]);
	}

	void cBuffer::pushDWord(const uint32_t pDWord) {
		dirty(true);
		const uint8_t* bytes = (const uint8_t*)& pDWord;

		push_back(bytes[0]);
		push_back(bytes[1]);
		push_back(bytes[2]);
		push_back(bytes[3]);
	}

	void cBuffer::pushBuffer(std::shared_ptr<cBuffer> pBuffer) {
		dirty(true);
		insert(end(), pBuffer->begin(), pBuffer->end());
	}

	void cBuffer::putByte(const size_t pOffset, uint8_t pByte) {
		expandOffset(pOffset);
		dirty(true);
		at(pOffset) = pByte;
	}

	void cBuffer::putWordLE(const size_t pOffset, uint16_t pByte) {
		expandOffset(pOffset, 2);
		dirty(true);
		uint16_t* bytes = reinterpret_cast<uint16_t*>(&at(pOffset));
		*bytes = pByte;
	}

	void cBuffer::putDWordLE(const size_t pOffset, uint32_t pByte) {
		expandOffset(pOffset, 4);
		dirty(true);
		uint32_t* bytes = reinterpret_cast<uint32_t*>(&at(pOffset));
		*bytes = pByte;
	}

	void cBuffer::putWordBE(const size_t pOffset, uint16_t pByte) {
		expandOffset(pOffset, 2);
		dirty(true);
		uint8_t* bytes = reinterpret_cast<uint8_t*>(&at(pOffset));
		bytes[0] = pByte >> 8;
		bytes[1] = pByte & 0xFF;
	}

	void cBuffer::putDWordBE(const size_t pOffset, uint32_t pByte) {
		expandOffset(pOffset, 4);
		dirty(true);
		uint8_t* bytes = reinterpret_cast<uint8_t*>(&at(pOffset));
		bytes[0] = pByte >> 24;
		bytes[1] = pByte >> 16;
		bytes[2] = pByte >> 8;
		bytes[3] = pByte & 0xFF;
	}

	void cBuffer::putBuffer(const size_t pOffset, std::shared_ptr<cBuffer> pBuffer) {
		expandOffset(pOffset, pBuffer->size());
		dirty(true);
		auto buf = data() + pOffset;
		memcpy(buf, pBuffer->data(), pBuffer->size());
	}

	void cBuffer::putString(const size_t pOffset, const std::string& pBuffer) {
		expandOffset(pOffset, pBuffer.size());
		dirty(true);
		auto buf = data() + pOffset;
		memcpy(buf, pBuffer.data(), pBuffer.size());
	}

	bool cBuffer::operator==(const cBuffer& pBuffer) {
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
		//assertOffset(pOffset, pBuffer->size());
		size_t maxSize = size() - pOffset;
		if (maxSize < pBuffer->size())
			return false;

		dirty(true);

		memcpy(&at(pOffset), pBuffer->data(), pBuffer->size());
		return true;
	}

	bool cBuffer::write(const size_t pOffset, const spBuffer pBuffer, const size_t pOffsetStart, const size_t pSize) {
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

}
