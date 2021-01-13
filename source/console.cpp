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

#include <memory>
#include <iomanip>
#include <iostream>
#include <time.h>
#include <sstream>
#include "console.hpp"

const char* TIME_FORMAT = "%Y-%m-%dT%H:%M:%S";

namespace firy {
	cConsole::cConsole(const int pLevel) {
		mLevel = pLevel;
	}

	std::string cConsole::getTime() const {
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
