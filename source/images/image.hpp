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
	namespace images {

		/**
		 * Image that contains a filesystem and a source interface
		 */
		class cImage : public virtual access::cInterface,
					   public filesystem::cInterface,
					   public std::enable_shared_from_this<cImage>,
					   public virtual firy::helpers::cDirty {

		public:

			cImage();
			~cImage();

			bool filesystemSave();

			/**
			 * Name of image type
			 */
			virtual std::string imageType() const = 0;

			/**
			 * Short name of image
			 */
			virtual std::string imageTypeShort() const = 0;

			/**
			 * Warning wrapper
			 */
			virtual spOptionResponse warning(const std::string& pMessage) {
				return mOptions->warning(this, pMessage);
			}

			/**
			 * Error wrapper
			 */
			virtual void error(const std::string& pMessage, const std::string& pMessageDetail = "") {
				mOptions->error(this, pMessage, pMessageDetail);
			}

			/**
			 * Save changes wrapper
			 */
			virtual spOptionResponse savechanges(const std::string& pMessage) {
				return mOptions->savechanges(this, pMessage);
			}

			/**
			 * Save changes due to exit
			 */
			virtual spOptionResponse savechangesExit(const std::string& pMessage) {
				return mOptions->savechangesExit(this, pMessage);
			}

			/**
			 * Set options
			 */
			virtual void optionsSet(spOptions pOptions);

			/**
			 * Create a file attached to this filesystem
			 */
			template <class tType, class ...Args> std::shared_ptr<tType> filesystemNodeCreate(const std::string& pName = "", ...) {
				auto res = std::make_shared<tType>(weak_from_this(), pName, Args...);
				res->dirty(true);
				return res;
			}

		private:
			spOptions mOptions;

		};

		/**
		 * Image with an access interface
		 */
		template <class tAccessInterface> class cImageAccess : 
			public cImage,
			public tAccessInterface {

		public:

			cImageAccess() : 
				cImage(), 
				tAccessInterface() {
			};
		};
	}

	using spImage = std::shared_ptr<images::cImage>;
	using wpImage = std::weak_ptr<images::cImage>;
}
