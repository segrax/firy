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

	namespace sources {

		/**
		 * Access a file
		 */
		class cFile : public cInterface {
		public:

			cFile(size_t pChunkSize = gMegabyte);

			virtual bool create(const std::string pFile) override;
			virtual bool open(const std::string pFile) override;
			virtual void close() override;
			virtual bool save(const std::string pFile) override;

			virtual spBuffer chunk(const size_t pFileOffset = 0) override;


		private:

		};

		/**
		 * File shared pointer
		 */
		typedef std::shared_ptr<cInterface> spSourceFile;

	}
}