
#include "firy.hpp"
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <filesystem>

namespace std {
	namespace filesystem = std::experimental::filesystem;
}

namespace firy {
	std::string cResources::getcwd() {
		return std::filesystem::current_path().string();
	}

	std::vector<std::string> cResources::directoryList(const std::string& pPath, const std::string& pExtension) {
		std::vector<std::string> files;

		std::filesystem::recursive_directory_iterator iter(pPath);
		std::filesystem::recursive_directory_iterator end;

		while (iter != end) {
			std::error_code ec;
			std::string file = iter->path().string();

			if(pExtension.size() == 0 || file.find(pExtension) != file.npos)
				files.push_back(file);

			iter.increment(ec);
			if (ec) {
				// TODO
				return files;
			}
		}

		return files;
	}

	spBuffer cResources::FileRead(const std::string& pFile, const size_t pOffset, const size_t pSize) {
		auto fileBuffer = std::make_shared<tBuffer>();

		// Attempt to open the file
		auto fileStream = new std::ifstream(pFile.c_str(), std::ios::binary);
		if (fileStream->is_open() != false) {
			fileStream->seekg(0, std::ios::end);
			size_t maxSize = fileStream->tellg();
			maxSize -= pOffset;
			fileStream->seekg(std::ios::beg);

			// Entire file?
			if (!pSize) {
				fileBuffer->resize(maxSize);
			} else {
				fileBuffer->resize( (pSize < maxSize) ? pSize : maxSize);
			}

			// Read from?
			if (pOffset)
				fileStream->seekg(pOffset, std::ios::beg);

			// Allocate buffer, and read the file into it
			fileStream->read((char*)fileBuffer->data(), fileBuffer->size());
			if (!(*fileStream))
				fileBuffer->clear();
		}

		// Close the stream
		fileStream->close();
		delete fileStream;

		fileBuffer->dirty(false);
		// All done ;)
		return fileBuffer;
	}

	bool cResources::FileWrite(const std::string& pFile, const size_t pOffset, spBuffer pBuffer) {
		std::ofstream outfile(pFile, std::ios::in | std::ios::out | std::ofstream::binary);
		if (!outfile.is_open())
			return false;
		outfile.seekp(pOffset, std::ios::beg);
		outfile.write((const char*)pBuffer->data(), pBuffer->size());
		outfile.close();
		pBuffer->dirty(false);
		return true;
	}

	bool cResources::FileSave(const std::string& pFile, const std::string& pData) {
		std::ofstream outfile(pFile, std::ofstream::binary);
		if (!outfile.is_open())
			return false;
		outfile << pData;
		outfile.close();
		return true;
	}

	bool cResources::FileSave(const std::string& pFile, const spBuffer pData) {
		std::ofstream outfile(pFile, std::ofstream::binary);
		if (!outfile.is_open())
			return false;
		outfile.write((const char*) pData->data(), pData->size());
		outfile.close();
		return true;
	}

	size_t cResources::FileSize(const std::string& pFile) const {
		std::streampos size = 0;
		auto fileStream = new std::ifstream(pFile.c_str(), std::ios::binary);
		if (fileStream->is_open()) {
			fileStream->seekg(0, std::ios::end);
			size = fileStream->tellg();
			fileStream->close();
		}
		return size;
	}

	bool cResources::FileExists(const std::string& pPath) const {
		struct stat info;

		if (stat(pPath.c_str(), &info) != 0)
			return false;
		else if (info.st_mode & S_IFDIR)
			return true;
		else if (info.st_mode & S_IFMT)
			return true;

		return false;
	}

	bool cResources::isFile(const std::string& pPath) const {
		struct stat info;

		if (stat(pPath.c_str(), &info) != 0)
			return false;
		else if (info.st_mode & S_IFDIR)
			return false;
		else if (info.st_mode & S_IFMT)
			return true;

		return false;
	}

}
