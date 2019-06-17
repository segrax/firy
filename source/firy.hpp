#include <iostream>
#include <memory>
#include <vector>
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
#include "images/image.hpp"

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

	inline void writeLEWord(const void* buffer, uint16_t pValue) {
		uint16_t* wordBytes = (uint16_t*)buffer;
		*wordBytes = pValue;
	}

	inline std::string str_to_lower(std::string pStr) {
		std::transform(pStr.begin(), pStr.end(), pStr.begin(), ::toupper);
		return pStr;
	}


	class cFiry {
	public:
		cFiry();
	};

	extern std::shared_ptr<cFiry> gFiry;
}
