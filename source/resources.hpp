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

	class cResources {

	public:
		std::string getcwd();
		std::vector<std::string> directoryList(const std::string& pPath, const std::string& pExtension);

		spBuffer FileRead(const std::string& pFile, const size_t pOffset = 0, const size_t pSize = 0);
		std::string	FileReadStr(const std::string& pFile);
		bool FileWrite(const std::string& pFile, const size_t pOffset, spBuffer pBuffer);
		
		bool FileSave(const std::string& pFile, const std::string& pData);
		bool FileSave(const std::string& pFile, const spBuffer pData);

		size_t FileSize(const std::string& pFile) const;
		bool FileExists(const std::string& pPath) const;
		bool isFile(const std::string& pPath) const;

	};
}
