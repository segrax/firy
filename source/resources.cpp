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

#include "firy.hpp"
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <filesystem>

namespace firy {

	namespace {
#ifdef _MSC_VER
		class cProcessFileLock {
		public:
			cProcessFileLock(const std::string& pFile, const bool pExclusive)
				: mLockValid(true), mHandle(INVALID_HANDLE_VALUE) {
				(void)pFile;
				(void)pExclusive;
			}

			~cProcessFileLock() {
				if (mHandle != INVALID_HANDLE_VALUE) {
					CloseHandle(mHandle);
				}
				mHandle = INVALID_HANDLE_VALUE;
			}

			bool valid() const { return mLockValid; }

		private:
			bool		mLockValid;
			HANDLE		mHandle;
			OVERLAPPED	mOverlap = {};
		};
#else
		class cProcessFileLock {
		public:
			cProcessFileLock(const std::string& pFile, const bool pExclusive) {}
			bool valid() const { return true; }
		};
#endif
	}

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

	std::string cResources::normalizePath(const std::string& pFile) const {
		if (pFile.empty())
			return pFile;

		try {
			auto abs = std::filesystem::absolute(pFile);
			std::error_code ec;
			auto canonical = std::filesystem::weakly_canonical(abs, ec);
			if (ec)
				return abs.string();

			return canonical.string();
		}
		catch (...) {
			return pFile;
		}
	}

	std::shared_ptr<std::shared_mutex> cResources::fileLock(const std::string& pFile) const {
		const auto file = normalizePath(pFile);
		std::lock_guard<std::mutex> lock(mFileMutex);

		auto it = mFileLocks.find(file);
		if (it != mFileLocks.end())
			return it->second;

		auto value = std::make_shared<std::shared_mutex>();
		mFileLocks.insert({ file, value });
		return value;
	}

	std::string cResources::FileReadStr(const std::string& pFile) {
		auto buffer = FileRead(pFile);
		if (!buffer || buffer->size() == 0)
			return {};

		return std::string((char*)buffer->data(), buffer->size());
	}

	spBuffer cResources::FileRead(const std::string& pFile, const size_t pOffset, const size_t pSize) {
		auto file = normalizePath(pFile);
		auto lock = fileLock(file);
		std::shared_lock<std::shared_mutex> threadLock(*lock);

#ifdef _MSC_VER
		cProcessFileLock processLock(file, false);
		if (!processLock.valid())
			return std::make_shared<tBuffer>();
#endif

		auto fileBuffer = std::make_shared<tBuffer>();

		std::ifstream fileStream(file, std::ios::binary);
		if (fileStream.is_open() == false)
			return fileBuffer;

		fileStream.seekg(0, std::ios::end);
		if (!fileStream)
			return fileBuffer;

		auto end = fileStream.tellg();
		if (end == -1)
			return fileBuffer;

		size_t maxSize = static_cast<size_t>(end);
		if (pOffset >= maxSize)
			return fileBuffer;

		maxSize -= pOffset;
		size_t readSize = pSize;
		if (!readSize || readSize > maxSize)
			readSize = maxSize;

		fileBuffer->resize(readSize);
		fileStream.seekg(pOffset, std::ios::beg);
		fileStream.read((char*)fileBuffer->data(), readSize);
		if (!fileStream) {
			auto read = static_cast<size_t>(fileStream.gcount());
			fileBuffer->resize(read);
		}

		fileBuffer->dirty(false);
		// All done ;)
		return fileBuffer;
	}

	bool cResources::FileWrite(const std::string& pFile, const size_t pOffset, spBuffer pBuffer) {
		if (!pBuffer)
			return false;

		auto file = normalizePath(pFile);
		auto lock = fileLock(file);
		std::unique_lock<std::shared_mutex> threadLock(*lock);

#ifdef _MSC_VER
		cProcessFileLock processLock(file, true);
		if (!processLock.valid())
			return false;
#endif

		if (!pBuffer->size()) {
			pBuffer->dirty(false);
			return true;
		}

		std::fstream outfile(file, std::ios::binary | std::ios::in | std::ios::out);
		if (!outfile.is_open()) {
			std::ofstream create(file, std::ios::binary);
			if (!create.is_open())
				return false;
			create.close();

			outfile.open(file, std::ios::binary | std::ios::in | std::ios::out);
			if (!outfile.is_open())
				return false;
		}

		outfile.seekp(pOffset, std::ios::beg);
		outfile.write((const char*)pBuffer->data(), pBuffer->size());
		outfile.flush();
		outfile.close();
		pBuffer->dirty(false);
		return true;
	}

	bool cResources::FileSave(const std::string& pFile, const std::string& pData) {
		auto file = normalizePath(pFile);
		auto lock = fileLock(file);
		std::unique_lock<std::shared_mutex> threadLock(*lock);

#ifdef _MSC_VER
		cProcessFileLock processLock(file, true);
		if (!processLock.valid())
			return false;
#endif

		std::ofstream outfile(file, std::ofstream::binary | std::ofstream::trunc);
		if (!outfile.is_open())
			return false;
		outfile << pData;
		outfile.close();
		return true;
	}

	bool cResources::FileSave(const std::string& pFile, const spBuffer pData) {
		if (!pData)
			return false;

		auto file = normalizePath(pFile);
		auto lock = fileLock(file);
		std::unique_lock<std::shared_mutex> threadLock(*lock);

#ifdef _MSC_VER
		cProcessFileLock processLock(file, true);
		if (!processLock.valid())
			return false;
#endif

		std::ofstream outfile(file, std::ofstream::binary | std::ofstream::trunc);
		if (!outfile.is_open())
			return false;
		outfile.write((const char*) pData->data(), pData->size());
		outfile.close();
		return true;
	}

	size_t cResources::FileSize(const std::string& pFile) const {
		auto file = normalizePath(pFile);
		auto lock = fileLock(file);
		std::shared_lock<std::shared_mutex> threadLock(*lock);

#ifdef _MSC_VER
		cProcessFileLock processLock(file, false);
		if (!processLock.valid())
			return 0;
#endif

		std::streampos size = 0;
		auto fileStream = std::ifstream(file.c_str(), std::ios::binary);
		if (fileStream.is_open()) {
			fileStream.seekg(0, std::ios::end);
			size = fileStream.tellg();
		}
		return size;
	}

	bool cResources::FileExists(const std::string& pPath) const {
		auto path = normalizePath(pPath);
		std::error_code ec;
		return std::filesystem::exists(std::filesystem::path(path), ec);
	}

	bool cResources::isFile(const std::string& pPath) const {
		auto path = normalizePath(pPath);
		std::error_code ec;
		return std::filesystem::is_regular_file(std::filesystem::path(path), ec);
	}

}
