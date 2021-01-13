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
	namespace filesystem {

		class sFile : public sNode {
		public:

			sFile(wpFilesystem pFilesystem, const std::string& pName = "");

			virtual spBuffer read();

			bool mChainBroken;
			std::vector<sAccessUnit> mChain;

			spBuffer mContent;
		};

		using spFile = std::shared_ptr<filesystem::sFile>;
	}
}
