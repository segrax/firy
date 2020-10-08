#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <algorithm>

#ifdef _MSC_VER
#include <Windows.h>
#endif

namespace firy {
	typedef size_t tSize;

	typedef std::vector<uint8_t> tBuffer;
	typedef std::shared_ptr<tBuffer> spBuffer;
}

#include "resources.hpp"
#include "sources/source.hpp"
#include "sources/file.hpp"

#include "interfaces/isource.hpp"
#include "interfaces/blocks.hpp"
#include "interfaces/tracks.hpp"

#include "filesystem/interface.hpp"
#include "filesystem/node.hpp"
#include "filesystem/file.hpp"
#include "filesystem/directory.hpp"

#include "images/disk.hpp"

namespace firy {

	extern std::shared_ptr<cResources> gResources;

	inline uint16_t readWord(const void* buffer) {
		const uint16_t* wordBytes = (const uint16_t*)buffer;
		return *wordBytes;
	}

	// Read a word from the buffer
	inline uint16_t readLEWord(const void* buffer) {
		const uint16_t* wordBytes = (const uint16_t*)buffer;
		return *wordBytes;
	}
	inline uint32_t readLEDWord(const void* buffer) {
		const uint32_t* wordBytes = (const uint32_t*)buffer;
		return *wordBytes;
	}

	inline uint16_t readBEWord(const void* buffer) {
		const uint8_t* bytes = (const uint8_t*)buffer;

		return uint16_t((bytes[0] << 8) + bytes[1]);
	}

	inline uint32_t readBEDWord(const void* buffer) {
		const uint8_t* bytes = (const uint8_t*)buffer;

		return uint32_t((bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + (bytes[3]));
	}

	inline void writeLEWord(const void* buffer, uint16_t pValue) {
		uint16_t* wordBytes = (uint16_t*)buffer;
		*wordBytes = pValue;
	}

	inline std::string str_to_lower(std::string pStr) {
		std::transform(pStr.begin(), pStr.end(), pStr.begin(), ::toupper);
		return pStr;
	}

	inline std::string stringRip(const uint8_t* pBuffer, uint8_t pTerminator, size_t pLengthMax) {
		std::string tmpString;

		for (size_t i = 0; *pBuffer != pTerminator && i <= pLengthMax; ++i)
			tmpString += (char)* pBuffer++;

		return tmpString;
	}

	static inline std::string ltrim(std::string s, uint8_t pChar) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [pChar](int ch) {
			return ch != pChar;
			}));
		return s;
	}
	static inline std::string rtrim(std::string s, uint8_t pChar) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [pChar](int ch) {
			return ch != pChar;
			}).base(), s.end());
		return s;
	}

	class cFiry {
	public:
		cFiry();
	};

	extern std::shared_ptr<cFiry> gFiry;
}
