namespace firy {

	class cBuffer : public std::vector<uint8_t> {
	public:
		uint8_t getByte(size_t pOffset);
		uint16_t getWordLE(size_t pOffset);
		uint32_t getDWordLE(size_t pOffset);

		void pushByte(uint8_t pByte);
		void pushWord(uint16_t pWord);
		void pushDWord(uint32_t pDWord);
		void pushBuffer(std::shared_ptr<cBuffer> pBuffer);
	};

	typedef cBuffer tBuffer;
	typedef std::shared_ptr<tBuffer> spBuffer;
}
