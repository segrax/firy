#include <memory>
#include <iomanip>
#include <iostream>
#include <time.h>
#include <sstream>
#include "debug.hpp"

const char* TIME_FORMAT = "%Y-%m-%dT%H:%M:%S";

namespace firy {
	cDebug::cDebug(const int pLevel) {
		mLevel = pLevel;
		mDisable = false;
	}

	std::string cDebug::getTime() const {
		std::stringstream res;

		auto t = std::time(nullptr);

#ifdef _WIN32
		tm tm;
		localtime_s(&tm, &t);
		res << std::put_time(&tm, TIME_FORMAT);
#else
		auto tm = *std::localtime(&t);
		res << std::put_time(&tm, TIME_FORMAT);
#endif
		return res.str();
	}
}
