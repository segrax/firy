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

#include <fstream>

namespace firy {

	class cConsole {

	public:
		cConsole(const int pLevel);

		std::string output() {
			return "\n";
		}

		void levelSet(const int pLevel) { mLevel = pLevel; }
		int levelGet() { return mLevel; }


		template<typename tMessage, typename ... tStrings>
		void error(const tMessage pModule, const tMessage pMessage, const tStrings ...pRest) {
			outputTime("error", pModule, pMessage, pRest...);;;
		}

		template<typename tMessage, typename ... tStrings>
		void notice(const tMessage pModule, const tMessage pMessage, const tStrings ...pRest) {
			if (mLevel >= 1)
				outputTime("notice", pModule, pMessage, pRest...);

			return { true };
		}

		/**
		 * Can abort
		 */
		template<typename tMessage, typename ... tStrings>
		void warning(const tMessage pModule, const tMessage pMessage, const tStrings ...pRest) {
			if (mLevel > 1)
				outputTime("warning", pModule, pMessage, pRest...);
		}

	private:

		template<typename tMessage, typename ... tStrings>
		std::string output(const tMessage pMessage, const tStrings& ...pRest) {
			return pMessage + output(pRest...);
		}

		template<typename tMessage, typename ... tStrings>
		void outputTime(const std::string pType, const tMessage pModule, const tMessage pMessage, const tStrings& ...pRest) {

			std::cerr << +": [" + pType + "] [" + pModule + "] " + output(pMessage, pRest...) << std::endl;
		}

		std::string getTime() const;

		int mLevel;
	};

	extern std::shared_ptr<firy::cConsole> gConsole;
}

