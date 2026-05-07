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

#include "images/d64.hpp"
#include "images/adf.hpp"
#include "images/fat.hpp"

#include <filesystem>

namespace {
	std::string sourceNormalizePath(const std::string& pPath) {
		if (pPath.empty()) {
			return pPath;
		}

		try {
			auto absolute = std::filesystem::absolute(pPath);
			std::error_code ec;
			auto canonical = std::filesystem::weakly_canonical(absolute, ec);
			if (ec)
				return absolute.string();
			return canonical.string();
		}
		catch (...) {
			return pPath;
		}
	}

	void pruneSourceCache(std::map<std::string, std::weak_ptr<firy::sources::cInterface>>& pCache) {
		for (auto it = pCache.begin(); it != pCache.end();) {
			if (it->second.expired()) {
				it = pCache.erase(it);
			}
			else {
				++it;
			}
		}
	}

	void pruneSourcePathLocks(std::map<std::string, std::weak_ptr<std::mutex>>& pCache) {
		for (auto it = pCache.begin(); it != pCache.end();) {
			if (it->second.expired()) {
				it = pCache.erase(it);
			}
			else {
				++it;
			}
		}
	}
}

namespace firy {

	std::shared_ptr<cResources> gResources;

	std::shared_ptr<cFiry> gFiry = std::make_shared<cFiry>();
	std::shared_ptr<cConsole> gConsole = std::make_shared<cConsole>(0);
	std::shared_ptr<cOptions> gOptionsDefault = std::make_shared<cOptions>();

	/**
	 *
	 */
	cFiry::cFiry() {
		gResources = std::make_shared<cResources>();
	}

	/**
	 * Get a shared lock for a normalized path
	 */
	std::shared_ptr<std::mutex> cFiry::sourcePathLock(const std::string& pPath) {
		std::lock_guard<std::mutex> lock(mSourceCacheMutex);
		pruneSourcePathLocks(mSourcePathLocks);

		auto it = mSourcePathLocks.find(pPath);
		if (it != mSourcePathLocks.end()) {
			auto lockPtr = it->second.lock();
			if (lockPtr) {
				return lockPtr;
			}

			mSourcePathLocks.erase(it);
		}

		auto lockPtr = std::make_shared<std::mutex>();
		mSourcePathLocks[pPath] = lockPtr;
		return lockPtr;
	}

	/**
	 * Create a file as a source
	 */
	spSource cFiry::createLocalFile(const std::string& pFilename) {
		const auto path = sourceNormalizePath(pFilename);
		auto pathLock = sourcePathLock(path);

		std::lock_guard<std::mutex> pathScopedLock(*pathLock);
		auto file = std::make_shared<firy::sources::cFile>();
		if (!file->create(pFilename)) {
			return 0;
		}

		{
			std::lock_guard<std::mutex> lock(mSourceCacheMutex);
			pruneSourceCache(mSourceCache);
			mSourceCache[path] = file;
		}
		return file;
	}

	/**
	 * Open a file as a source
	 */
	spSource cFiry::openLocalFile(const std::string& pFilename) {
		const auto path = sourceNormalizePath(pFilename);
		auto pathLock = sourcePathLock(path);

		std::lock_guard<std::mutex> pathScopedLock(*pathLock);
		{
			std::lock_guard<std::mutex> lock(mSourceCacheMutex);
			pruneSourceCache(mSourceCache);

			auto it = mSourceCache.find(path);
			if (it != mSourceCache.end()) {
				auto source = it->second.lock();
				if (source) {
					return source;
				}
				mSourceCache.erase(it);
			}
		}

		auto file = std::make_shared<firy::sources::cFile>();
		if (!file->open(pFilename)) {
			return 0;
		}

		{
			std::lock_guard<std::mutex> lock(mSourceCacheMutex);
			pruneSourceCache(mSourceCache);

			auto it = mSourceCache.find(path);
			if (it != mSourceCache.end()) {
				auto source = it->second.lock();
				if (source) {
					return source;
				}
				mSourceCache.erase(it);
			}
			mSourceCache[path] = file;
		}

		return file;
	}

	/**
	 * Open an image from a local file
	 */
	template <class tImageType> std::shared_ptr<tImageType> cFiry::openImageFile(const std::string& pFilename, spOptions pOptions, const bool pIgnoreValid) {
		return openImageFile<tImageType>(openLocalFile(pFilename), pOptions, pIgnoreValid);
	}

	/**
	 * Open an image from an already opened source
	 */
	template <class tImageType> std::shared_ptr<tImageType> cFiry::openImageFile(spSource pSource, spOptions pOptions, const bool pIgnoreValid) {
		auto image = std::make_shared<tImageType>(pSource);
		if (image)
			image->optionsSet(pOptions);

		if (!image || image->filesystemLoad() == false && !pIgnoreValid) {
			return 0;
		}

		return image;
	}

	/**
	  * Get a list of known file extensions
	  */
	std::vector<std::string> cFiry::getKnownExtensions() {
		std::vector<std::string> extensions;

		for (auto& ext : images::cD64::imageExtensions()) {
			extensions.push_back(ext);
		}
		for (auto& ext : images::cADF::imageExtensions()) {
			extensions.push_back(ext);
		}
		for (auto& ext : images::cFAT::imageExtensions()) {
			extensions.push_back(ext);
		}
		return extensions;
	}

	/**
	 *
	 */
	spImage cFiry::openImage(const std::string& pFilename) {
		spImage file = 0;

		spOptions options = gOptionsDefault->clone();
		options->errorShowSet(false);

		auto source = openLocalFile(pFilename);
		if (!source)
			return 0;

		try {
			// D64
			if (images::cD64::imageTest(source)) {
				file = openImageFile<images::cD64>(source, options);
			}
		} catch (std::exception exception) {
		}

		// ADF
		try {
			if (!file && images::cADF::imageTest(source)) {
				file = openImageFile<images::cADF>(source, options);
			}
		} catch (std::exception exception) {
		}

		// FAT
		try {
			if (!file && images::cFAT::imageTest(source)) {
				file = openImageFile<images::cFAT>(source, options);
			}
		} catch (std::exception exception) {
		}

		return file;
	}
}
