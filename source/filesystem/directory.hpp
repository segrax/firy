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
		class sDirectory : public sNode {
		public:
			sDirectory(wpFilesystem pFilesystem, const std::string& pName = "");

			virtual spNode getByName(const std::string& pName, const bool pCaseSensitive = false) override;

			virtual void nodeAdd(spNode pNode);
			virtual void nodeRemove(spNode pNode);

			virtual bool add(spNode pNode);			// Add to the file system
			virtual bool remove() override;			// Remove from the file system
			virtual bool isDirectory() const override { return true; }

			std::vector<spNode> mNodes;
		};
	}

	using spDirectory = std::shared_ptr<filesystem::sDirectory>;
}
