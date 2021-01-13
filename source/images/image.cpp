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

#include "firy.hpp"

namespace firy {
	namespace images {

		/**
		 * Constructor
		 */
		cImage::cImage() :
			access::cInterface(0),
			filesystem::cInterface(),
			std::enable_shared_from_this<cImage>() {

			optionsSet(gOptionsDefault);
		};

		/**
		 *
		 */
		cImage::~cImage() {

			if (sourceIsDirty() && !savechangesExit("Save Changes")->isAborted()) {
				sourceSave();
			}
		}

		/**
		 * Save the filesystem, and trigger a source save
		 */
		bool cImage::filesystemSave() {
			auto res = filesystemSaveNative();
			if (!res)
				return false;

			if (sourceIsDirty() && !savechanges("Save Changes")->isAborted()) {
				sourceSave();
			}
			return true;
		}

		/**
		 * Options
		 */
		void cImage::optionsSet(spOptions pOptions) {
			mOptions = pOptions->clone();
		}

	}
}
