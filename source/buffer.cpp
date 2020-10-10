#include <vector>
#include <memory>
#include "buffer.hpp"

namespace firy {

	/**
	 *
	 */
	uint8_t cBuffer::getByte(size_t pOffset) {

		return at(pOffset);
	}

	/** 
	 * TODO: Need endian switch around these functions for platform
	 */


	/**
	 * Get a word, little-endian
	 */
	uint16_t cBuffer::getWordLE(size_t pOffset) {
		const uint16_t* bytes = reinterpret_cast<uint16_t*>(&at(pOffset));
		return *bytes;
	}

	/**
	 * Get a double-word, little-endian
	 */
	uint32_t cBuffer::getDWordLE(size_t pOffset) {
		const uint32_t* bytes = reinterpret_cast<uint32_t*>(&at(pOffset));
		return *bytes;
	}

	void cBuffer::pushByte(uint8_t pByte) {
		push_back(pByte);
	}

	void cBuffer::pushWord(uint16_t pWord) {
		const uint8_t* bytes = (const uint8_t*)& pWord;

		push_back(bytes[0]);
		push_back(bytes[1]);
	}

	void cBuffer::pushDWord(uint32_t pDWord) {
		const uint8_t* bytes = (const uint8_t*)& pDWord;

		push_back(bytes[0]);
		push_back(bytes[1]);
		push_back(bytes[2]);
		push_back(bytes[3]);
	}

	void cBuffer::pushBuffer(std::shared_ptr<cBuffer> pBuffer) {

		insert(end(), pBuffer->begin(), pBuffer->end());
	}

}
