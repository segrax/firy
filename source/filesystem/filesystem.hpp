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
		class sNode;
		class sFile;
		class sDirectory;
	}
	using spNode = std::shared_ptr<filesystem::sNode>;
	using wpNode = std::weak_ptr<filesystem::sNode>;
	using wpDirectory = std::weak_ptr<filesystem::sDirectory>;

	using spFile = std::shared_ptr<filesystem::sFile>;
	using spDirectory = std::shared_ptr<filesystem::sDirectory>;

	namespace filesystem {
		/**
		 * Provide a filesystem interface
		 */
		class cInterface : public virtual firy::helpers::cDirty {
		public:
			cInterface();

			/**
			 * Create a file in the filesystem native class, returning a generic pointer
			 */
			virtual spFile filesystemFileCreate(const std::string& pName = "") = 0;

			/**
			 * Create a directory in the filesystem native class, returning a generic pointer
			 */
			virtual spDirectory filesystemDirectoryCreate(const std::string& pName = "") = 0;

			virtual std::string filesystemNameGet() const;
			virtual void filesystemNameSet(const std::string& pName);

			virtual spNode filesystemNode(const std::string& pPath);
			virtual spDirectory filesystemPath(const std::string& pPath = "/");
			virtual spFile filesystemFile(const std::string& pPath);

			virtual spBuffer filesystemRead(spNode pFile) = 0;
			virtual bool filesystemRemove(spNode pFile) = 0;

			virtual bool filesystemCreate() = 0;
			virtual bool filesystemLoad() = 0;
			virtual bool filesystemSave() = 0;

			virtual size_t filesystemTotalBytesFree() = 0;
			virtual size_t filesystemTotalBytesMax() = 0;

		protected:

			virtual bool filesystemChainLoad(spFile pFile) = 0;
			virtual bool filesystemBitmapLoad() = 0;
			virtual bool filesystemBitmapSave() = 0;

			virtual bool filesystemSaveNative() = 0;

			spDirectory mFsRoot;
			std::string mFsPathSeperator;
			std::string mFsName;
		};
	}

	using spFilesystem = std::shared_ptr<filesystem::cInterface>;
	using wpFilesystem = std::weak_ptr<filesystem::cInterface> ;
}
