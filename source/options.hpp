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
	class cOptions;

	/**
	 * Response to a failure
	 */
	class cOptionResponse {
	public:
		inline bool isAborted() const { return mAbort; }
		cOptionResponse(bool pAbort) { mAbort = pAbort; }

	protected:
		bool mAbort;
	};

	using spOptionResponse = std::shared_ptr<cOptionResponse>;
	using spOptions = std::shared_ptr<cOptions>;
	
	/**
	 * User options
	 */
	class cOptions : public std::enable_shared_from_this<cOptions> {

	public:
		cOptions();

		virtual spOptionResponse warning(pImage pImage, const std::string& pMessage, const std::string& pMessageDetail = "");
		virtual void error(pImage pImage, const std::string& pMessage, const std::string& pMessageDetail = "");
		virtual spOptionResponse savechanges(pImage pImage, const std::string& pMessage);
		virtual spOptionResponse savechangesExit(pImage pImage, const std::string& pMessage);


		virtual void errorShowSet(const bool pEnabled);
		virtual void warningShowSet(const bool pEnabled);


		virtual spOptions clone() { return std::make_shared<cOptions>(*shared_from_this()); };

	protected:
		bool mAutoSaveSource;
		bool mAutoSaveSourceExit;
		bool mWarningShow;
		bool mErrorShow;

	private:
		
	};


}
