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

		/**
		 *
		 */
		sNode::sNode(wpFilesystem pFilesystem, const std::string& pName) {
			mFilesystem = pFilesystem;
			mName = pName;
			mSizeInBytes = 0;
		}

		/**
		 *
		 */
		spNode sNode::getByName(const std::string& pName, const bool pCaseSensitive) {
			return 0;
		}

		/**
		 * Remove
		 */
		bool sNode::remove() {
			if (!mParent.expired())
				mParent.lock()->nodeRemove(shared_from_this());

			mFilesystem.lock()->filesystemRemove(shared_from_this());
			return mFilesystem.lock()->filesystemSave();
		}

		/**
		 *
		 */
		sFile::sFile(wpFilesystem pFilesystem, const std::string& pName) : sNode(pFilesystem, pName) {
			mChainBroken = false;
			mSizeInBytes = 0;
		}

		/**
		 *
		 */
		spBuffer sFile::read() {
			return mFilesystem.lock()->filesystemRead(shared_from_this());
		}

		/**
		 *
		 */
		sDirectory::sDirectory(wpFilesystem pFilesystem, const std::string& pName) : sNode(pFilesystem, pName) {

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
					if (str_to_upper(node->mName) == str_to_upper(pName))
						return node;
				}
			}

			return 0;
		}

		/**
		 * Add to the file system
		 */
		bool sDirectory::add(spNode pNode) {
			nodeAdd(pNode);
			mFilesystem.lock()->dirty();
			dirty();
			return mFilesystem.lock()->filesystemSave();
		}

		/**
		 * Remove from the file system
		 */
		bool sDirectory::remove() {
			dirty();
			for (auto& node : mNodes) {
				node->mParent.reset();
				if (!node->remove())
					return false;
			}

			mNodes.clear();
			return sNode::remove();
		}

		/**
		 * Add an existing node to the directory (No FS calls)
		 */
		void sDirectory::nodeAdd(spNode pNode) {
			pNode->mParent = std::dynamic_pointer_cast<sDirectory>(shared_from_this());

			mNodes.push_back(pNode);
		}

		/**
		 * Remove a node from this directory (no FS Calls)
		 */
		void sDirectory::nodeRemove(spNode pNode) {
			for (auto it = mNodes.begin(); it != mNodes.end(); ++it) {
				if (*it == pNode) {

					mNodes.erase(it);
					return;
				}
			}
		}

		/**
		 *
		 */
		cInterface::cInterface() {
			mFsRoot = 0;
			mFsPathSeperator = "/";
		}

		/**
		 * Find a node inside the FS
		 */
		spNode cInterface::filesystemNode(const std::string& pPath) {
			auto nodesremain = tokenize(pPath, mFsPathSeperator);
			spNode node = mFsRoot;

			for (auto& remain : nodesremain) {
				node = node->getByName(remain);
				if (!node)
					break;
			}

			return node;
		}

		/**
		 * Get a path inside the FS
		 */
		spDirectory cInterface::filesystemPath(const std::string& pPath) {
			return std::dynamic_pointer_cast<sDirectory>(filesystemNode(pPath));
		}

		/**
		 * Get a file inside the FS
		 */
		spFile cInterface::filesystemFile(const std::string& pPath) {
			return std::dynamic_pointer_cast<sFile>(filesystemNode(pPath));
		}

		/**
		 * Get the name of the filesystem (Label)
		 */
		std::string cInterface::filesystemNameGet() const { 
			return mFsName;
		}

		/**
		 * Set the name of the filesystem
		 */
		void cInterface::filesystemNameSet(const std::string& pName) {
			mFsName = pName;
			dirty();
		}

	}
}
