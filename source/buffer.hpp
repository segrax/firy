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

	class cBuffer;
	using tBuffer = cBuffer;
	using spBuffer = std::shared_ptr<tBuffer>;

	class cBuffer : public firy::helpers::cDirty, protected std::vector<uint8_t> {
	public:

		spBuffer takeBytes(const size_t pBytes);

		std::string getString(const size_t pOffset, const size_t pLengthMax, const uint8_t pTerminator = 0);

		uint8_t getByte(const size_t pOffset) const;
		uint16_t getWordLE(const size_t pOffset) const;
		uint32_t getDWordLE(const size_t pOffset) const;

		uint16_t getWordBE(const size_t pOffset) const;
		uint32_t getDWordBE(const size_t pOffset) const;

		void pushByte(const uint8_t pByte);
		void pushWord(const uint16_t pWord);
		void pushDWord(const uint32_t pDWord);
		void pushBuffer(std::shared_ptr<cBuffer> pBuffer);
		bool pushBuffer(std::shared_ptr<cBuffer> pBuffer, const size_t pSourceOffset, const size_t pMax);
		void pushBuffer(std::shared_ptr<cBuffer> pBuffer, const size_t pMax);
		void pushBuffer(const uint8_t* pBuffer, const size_t pSize);

		void putByte(const size_t pOffset, uint8_t pByte);
		void putWordLE(const size_t pOffset, uint16_t pByte);
		void putDWordLE(const size_t pOffset, uint32_t pByte);

		void putWordBE(const size_t pOffset, uint16_t pByte);
		void putDWordBE(const size_t pOffset, uint32_t pByte);

		void putBuffer(const size_t pOffset, std::shared_ptr<cBuffer> pBuffer);
		void putString(const size_t pOffset, const std::string& pBuffer);

		bool operator==(const cBuffer& pBuffer);
		bool operator!=(const cBuffer& pBuffer);

		bool write(const size_t pOffset, const spBuffer pBuffer);
		bool write(const size_t pOffset, const spBuffer pBuffer, const size_t pOffsetStart, const size_t pSize);
		bool write(const size_t pOffset, const uint8_t* pBuffer, const size_t pSize);

		void resize(const size_type pNewSize) {
			tLockGuard lock(mLock);
			std::vector<uint8_t>::resize(pNewSize);
			dirty();
		}

		/**
		 * These are wrappers and should probably be replaced/not used
		 */
		uint8_t& at(const size_t pOffset)  { return std::vector<uint8_t>::at(pOffset); }
		const uint8_t& at(const size_t pOffset) const { return std::vector<uint8_t>::at(pOffset); }
		uint8_t* data() { return &at(0); }
		void clear() { tLockGuard lock(mLock); std::vector<uint8_t>::clear(); }
		size_t size() const { return std::vector<uint8_t>::size(); }
		
	private:
		inline void assertOffset(const size_t pOffset, const size_t pBytes = 1) const;
		inline void expandOffset(const size_t pOffset, const size_t pBytes = 1);
		mutable std::mutex mLock;
	};

}
