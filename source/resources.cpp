
#include "firy.hpp"
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>

namespace firy {
	spBuffer cResources::FileRead(const std::string& pFile) {
		auto fileBuffer = std::make_shared<tBuffer>();

		// Attempt to open the file
		auto fileStream = new std::ifstream(pFile.c_str(), std::ios::binary);
		if (fileStream->is_open() != false) {

			// Get file size
			fileStream->seekg(0, std::ios::end);
			fileBuffer->resize(static_cast<const unsigned int>(fileStream->tellg()));
			fileStream->seekg(std::ios::beg);

			// Allocate buffer, and read the file into it
			fileStream->read((char*)fileBuffer->data(), fileBuffer->size());
			if (!(*fileStream))
				fileBuffer->clear();
		}

		// Close the stream
		fileStream->close();
		delete fileStream;

		// All done ;)
		return fileBuffer;
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
