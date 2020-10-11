#include <vector>
#include <memory>
#include <string>
#include "buffer.hpp"

namespace firy {

	std::string cBuffer::getString(const size_t pOffset, const size_t pLengthMax, const uint8_t pTerminator) {
		std::string tmpString;
		const uint8_t* buffer = reinterpret_cast<const uint8_t*>(&at(pOffset));

		for (size_t i = 0; *buffer != pTerminator && i <= pLengthMax; ++i)
			tmpString += (char)* buffer++;

		return tmpString;
	}

	/**
	 * Get a byte
	 */
	uint8_t cBuffer::getByte(size_t pOffset) const {

		return at(pOffset);
	}

	/** 
	 * TODO: Need endian switch around these functions for platform
	 */


	/**
	 * Get a word, little-endian
	 */
	uint16_t cBuffer::getWordLE(const size_t pOffset) const {
		const uint16_t* bytes = reinterpret_cast<const uint16_t*>(&at(pOffset));
		return *bytes;
	}

	/**
	 * Get a double-word, little-endian
	 */
	uint32_t cBuffer::getDWordLE(const size_t pOffset) const {
		const uint32_t* bytes = reinterpret_cast<const uint32_t*>(&at(pOffset));
		return *bytes;
	}

	/**
	 * Get a word, big-endian
	 */
	uint16_t cBuffer::getWordBE(const size_t pOffset) const {
		const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&at(pOffset));
		return uint16_t((bytes[0] << 8) + bytes[1]);
	}

	/**
	 * Get a double-word, big-endian
	 */
	uint32_t cBuffer::getDWordBE(const size_t pOffset) const {
		const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&at(pOffset));
		return uint32_t((bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + (bytes[3]));
	}

	void cBuffer::pushByte(const uint8_t pByte) {
		push_back(pByte);
	}

	void cBuffer::pushWord(const uint16_t pWord) {
		const uint8_t* bytes = (const uint8_t*)& pWord;

		push_back(bytes[0]);
		push_back(bytes[1]);
	}

	void cBuffer::pushDWord(const uint32_t pDWord) {
		const uint8_t* bytes = (const uint8_t*)& pDWord;

		push_back(bytes[0]);
		push_back(bytes[1]);
		push_back(bytes[2]);
		push_back(bytes[3]);
	}

	void cBuffer::pushBuffer(std::shared_ptr<cBuffer> pBuffer) {

		insert(end(), pBuffer->begin(), pBuffer->end());
	}

	bool cBuffer::operator==(const cBuffer& pBuffer) {
		if (size() != pBuffer.size())
			return false;
		return std::equal(begin(), end(), pBuffer.begin());
	}

	bool cBuffer::operator!=(const cBuffer& pBuffer) {
		return !this->operator==(pBuffer);
	}
}
