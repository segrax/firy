namespace firy {

	class cBuffer : public std::vector<uint8_t> {
	public:

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
	};

	typedef cBuffer tBuffer;
	typedef std::shared_ptr<tBuffer> spBuffer;
}
