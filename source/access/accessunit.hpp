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

	typedef size_t tTrack;
	typedef size_t tSector;
	typedef size_t tBlock;

	typedef std::pair<tTrack, tSector> tTrackSector;

	/**
	 * Represent a lowlevel unit on an access device
	 */
	struct sAccessUnit {
		tTrackSector mTS;
		tBlock mBlock;
		bool mTrackBased;

		sAccessUnit() { mTS.first = 0; mTS.second = 0; mBlock = 0; }
		sAccessUnit(const tTrackSector& pTS) { mTS = pTS; mTrackBased = true; }
		sAccessUnit(const tBlock& pBlock) { mBlock = pBlock; }
		sAccessUnit(const sAccessUnit& pChain) { mTS = pChain.mTS; mTrackBased = pChain.mTrackBased; mBlock = pChain.mBlock; }

		void operator=(const sAccessUnit& pChain) { mTS = pChain.mTS; mBlock = pChain.mBlock; }
		tTrack track() const { return mTS.first; }
		tSector sector() const { return mTS.second; }
		tBlock block() const { return mBlock; }

		bool isTrackBased() { return mTrackBased; }
	};

	using tFreeBlocks = sAccessUnit;
}
