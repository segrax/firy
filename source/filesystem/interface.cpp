#include "firy.hpp"

namespace firy {
	namespace filesystem {

		std::vector<std::string> tokenize(const std::string& in, const std::string& delim) {
			std::vector<std::string> tokens;

			std::string::size_type pos_begin, pos_end = 0;
			std::string input = in;

			while ((pos_begin = input.find_first_not_of(delim, pos_end)) != std::string::npos) {
				pos_end = input.find_first_of(delim, pos_begin);
				if (pos_end == std::string::npos) pos_end = input.length();

				tokens.push_back(input.substr(pos_begin, pos_end - pos_begin));
			}

			return tokens;
		}

		sNode::sNode(wpFilesystem pFilesystem) {
			mFilesystem = pFilesystem;
		}

		spNode sNode::getByName(const std::string& pName, const bool pCaseSensitive) {
			return 0;
		}

		sFile::sFile(wpFilesystem pFilesystem) : sNode(pFilesystem) {
			mChainBroken = false;
			mSizeInBytes = 0;
		}

		spBuffer sFile::read() {
			return mFilesystem.lock()->filesystemRead(shared_from_this());
		}

		sDirectory::sDirectory(wpFilesystem pFilesystem) : sNode(pFilesystem) {

		}

		/**
		 *
		 */
		spNode sDirectory::getByName(const std::string& pName, const bool pCaseSensitive) {

			for (auto& node : mNodes) {
				if (pCaseSensitive) {
					if (node->mName == pName)
						return node;

				} else {
					if (str_to_lower(node->mName) == str_to_lower(pName))
						return node;
				}
			}

			return 0;
		}

		cInterface::cInterface() {
			mFsRoot = 0;
		}

		spNode cInterface::filesystemNode(const std::string& pPath) {
			auto nodesremain = tokenize(pPath, "/");
			spNode node = mFsRoot;

			for (auto& remain : nodesremain) {
				node = node->getByName(remain);
				if (!node)
					break;
			}

			return node;
		}

		spDirectory cInterface::filesystemPath(const std::string& pPath) {
			return std::dynamic_pointer_cast<sDirectory>(filesystemNode(pPath));
		}

		spFile cInterface::filesystemFile(const std::string& pPath) {
			return std::dynamic_pointer_cast<sFile>(filesystemNode(pPath));
		}
	}
}
