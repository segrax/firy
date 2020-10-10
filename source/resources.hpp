namespace firy {

	class cResources {

	public:
		std::string getcwd();
		std::vector<std::string> directoryList(const std::string& pPath, const std::string& pExtension);

		spBuffer FileRead(const std::string& pFile, const size_t pOffset = 0, const size_t pSize = 0);
		std::string	FileReadStr(const std::string& pFile);

		bool FileSave(const std::string& pFile, const std::string& pData);
		bool FileSave(const std::string& pFile, const spBuffer pData);

		size_t FileSize(const std::string& pFile) const;
		bool FileExists(const std::string& pPath) const;
		bool isFile(const std::string& pPath) const;

	};
}
