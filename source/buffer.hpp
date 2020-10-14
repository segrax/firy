namespace firy {

	class cBuffer;
	typedef cBuffer tBuffer;
	typedef std::shared_ptr<tBuffer> spBuffer;

	class cBuffer : public firy::helpers::cDirty, public std::vector<uint8_t> {
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
			std::vector<uint8_t>::resize(pNewSize);
			dirty();
		}

	private:
		inline void assertOffset(const size_t pOffset, const size_t pBytes = 1) const;
		inline void expandOffset(const size_t pOffset, const size_t pBytes = 1);

	};

}
