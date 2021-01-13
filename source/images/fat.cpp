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
#include "fat.hpp"

namespace firy {
	namespace images {
		namespace fat {

			sFile::sFile(wpFilesystem pFilesystem, const std::string& pName) : sEntry(), filesystem::sFile(pFilesystem, pName) {

			}

			sDir::sDir(wpFilesystem pFilesystem, const std::string& pName) : sEntry(), filesystem::sDirectory(pFilesystem, pName) {

			}

			std::vector<uint8_t> gPartitionTypes_FAT = {
				0x01, //DOS 12-bit fat
				0x04, //DOS 3.0+ 16-bit FAT (up to 32M)
				0x05, //DOS 3.3+ Extended Partition
				0x06, //DOS 3.31+ 16-bit FAT (over 32M)
				0x0b, //WIN95 OSR2 32-bit FAT
				0x0c, //WIN95 OSR2 32-bit FAT, LBA-mapped
				0x0e, //WIN95: DOS 16-bit FAT, LBA-mapped
				0x0f, //WIN95: Extended partition, LBA-mapped
				0x11, //Hidden DOS 12-bit FAT
				0x14, //Hidden DOS 16-bit FAT <32M
				0x16, //Hidden DOS 16-bit FAT >=32M
				0x1b, //Hidden WIN95 OSR2 32-bit FAT
				0x1c, //Hidden WIN95 OSR2 32-bit FAT, LBA-mapped
				0x1e, //Hidden WIN95 16-bit FAT, LBA-mapped
			};

			bool partitionTypeValid(uint8_t pType) {
				for (auto type : gPartitionTypes_FAT) {
					if (type == pType)
						return true;
				}
				return false;
			}
		}

		cFAT::cFAT(spSource pSource) : cImageAccess<access::cBlocks>(), access::cInterface(pSource) {

			// Set a static blocksize
			mBlockSize = 512;	
			mBlockPartitionStart = 0;
			mBlockFAT = 0;
			mBlockRoot = 0;
			mType = fat::eType::eType_Unknown;
		}

		tBlock cFAT::fatSectorNext(tBlock pCluster) const {
			if (pCluster > mClusterMap.size()) {
				//std::cout << "Invalid Cluster\n";
				return 0;
			}

			auto next = mClusterMap[pCluster];
			if ((next == 0) || 
				(mType == fat::eType_FAT12 && next >= 0x0FF0 && next <= 0x0FFF) ||
				(mType == fat::eType_FAT16 && next >= 0xFFF0 && next <= 0xFFFF) ||
				(mType == fat::eType_FAT32 && next >= 0x0FFFFFF0 && next <= 0x0FFFFFFF))
				return 0;
			
			return next & 0x0FFFFFFF;
		}

		tBlock cFAT::directorySectors(tBlock pStart) const {
			tBlock totalClusters = 1, cl;

			cl = pStart;
			while (pStart) {
				pStart = fatSectorNext(pStart);

				if(pStart)
					totalClusters++;
			}

			return totalClusters * mBootBlock->mBiosParams.mSectorsPerCluster;
		}

		/**
		 * Convert a cluster number to a block
		 */
		tBlock cFAT::clusterToBlock(tBlock pCluster) const {
			return mBlockData + ((pCluster - 2) * mBootBlock->mBiosParams.mSectorsPerCluster);
		}

		/**
		 * 
		 */
		std::string cFAT::filesystemNameGet() const {
			if (mLabel.size())
				return mLabel;

			return std::string(mBootBlock->mBiosParam.mLabel, strlen(mBootBlock->mBiosParam.mLabel));
		}

		/**
		 *
		 */
		bool cFAT::filesystemLoad() {
			auto bootBlock = blockObjectGet<fat::sBootRecordBlock>(0);
			if (!bootBlock)
				return false;

			fat::sParitionTableEntry* other = 0;

			// Check the partition table
			for (int part = 0; part < 4; ++part) {

				if (bootBlock->mMasterBootRecord.mPartitions[part].mType == 0)
					continue;

				if (!fat::partitionTypeValid(bootBlock->mMasterBootRecord.mPartitions[part].mType))
					continue;

				// Active?
				if (bootBlock->mMasterBootRecord.mPartitions[part].mActive == 0x80) {
					return partitionOpen(bootBlock->mMasterBootRecord.mPartitions[part].mStartLBA);
				}

				other = &bootBlock->mMasterBootRecord.mPartitions[part];
			}

			if (other) {
				return partitionOpen(other->mStartLBA);
			}
			return partitionOpen(0);
		}

		/**
		 *
		 */
		bool cFAT::partitionOpen(int pNumber) {
			mBootBlock = blockObjectGet<fat::sBootRecordBlock>(pNumber);
			if (!mBootBlock)
				return false;

			// Some really old disks dont have the signature (eg. Wang 3)
			//if (mBootBlock->mSignature1 != 0x55 || mBootBlock->mSignature2 != 0xAA)
			//	return false;

			mBlockPartitionStart = pNumber;
			mBlockFAT = mBlockPartitionStart + mBootBlock->mBiosParams.mSectorsReserved;

			//eType_FAT12 / eType_FAT16
			if (mBootBlock->mBiosParams.mSectorsPerFAT && mBootBlock->mBiosParams.mSectorsPerCluster <= 128) {
				if (mBootBlock->mBiosParams.mRootEntryCount) {
					mBlockRoot = (mBlockFAT + (mBootBlock->mBiosParams.mSectorsPerFAT * 2));
					mBlockData = mBlockRoot + (((mBootBlock->mBiosParams.mRootEntryCount * sizeof(fat::sFileEntry)) + (512 - 1)) / 512);

					mClusterRoot = mBlockRoot / mBootBlock->mBiosParams.mSectorsPerCluster;
				}
			} else {
				if (mBootBlock->mBiosParam32.mFatTotalSectors) {
					mBlockData = mBootBlock->mBiosParams.mSectorsHidden + ((mBootBlock->mBiosParam32.mFatTotalSectors * mBootBlock->mBiosParams.mFatCount) + mBootBlock->mBiosParams.mSectorsReserved);
					mBlockRoot = mBlockData + (mBlockRoot * mBootBlock->mBiosParams.mSectorsPerCluster);
				
					mClusterRoot = blockToCluster(mBlockRoot);
				}
			}

			if (mBootBlock->mBiosParams.mSectorsPerCluster) {
				mClustersTotal = (blockCount() - mBlockData) / mBootBlock->mBiosParams.mSectorsPerCluster;
				if (mClustersTotal < 4085 && mBootBlock->mBiosParams.mSectorsTotal)
					mType = fat::eType_FAT12;
				else if (mClustersTotal < 65525 && (mBootBlock->mBiosParams.mSectorsTotal || mBootBlock->mBiosParams.mSectorsTotal_H) && !strncmp((const char*)mBootBlock->mBiosParam.mFilesystemID, "FAT16", 5))
					mType = fat::eType_FAT16;
				else if (mClustersTotal <= 268435455 && !mBootBlock->mBiosParams.mSectorsTotal && mBootBlock->mBiosParams.mSectorsTotal_H && !strncmp((const char*)mBootBlock->mBiosParam32.mFilesystemID, "FAT32", 5))
					mType = fat::eType_FAT32;
			}

			if (mType == fat::eType_Unknown || !filesystemBitmapLoad()) {
				return false;
			}

			auto Root = std::make_shared<fat::sDir>(weak_from_this());
			Root->mFirstCluster = mClusterRoot;
			Root->mBlock = mBlockRoot;
			Root->mSizeInBytes = mBootBlock->mBiosParams.mRootEntryCount * sizeof(fat::sFileEntry);

			if (!Root->mSizeInBytes) {
				Root->mSizeInBytes = directorySectors(blockToCluster(Root->mBlock)) * blockSize();
			}
			mFsRoot = Root;
			return entrysLoad(Root);
		}

		/**
		 * Read and follow a chain of clusters, starting at an LBA
		 */
		spBuffer cFAT::clusterChainReadRoot(size_t pStartBlock) {
			size_t totalbytes = (mBootBlock->mBiosParams.mRootEntryCount * sizeof(fat::sFileEntry));
			auto buffer = std::make_shared<tBuffer>();
			size_t remainbytes = totalbytes;

			while (remainbytes) {
				auto size = min(remainbytes, mBootBlock->mBiosParams.mSectorsPerCluster * blockSize());
				auto block = sourceBufferCopy(pStartBlock * blockSize(), size);
				if (!block)
					return 0;

				buffer->pushBuffer(block, size);
				remainbytes -= size;

				pStartBlock += mBootBlock->mBiosParams.mSectorsPerCluster;
			}
			buffer->resize(totalbytes);
			return buffer;
		}

		/**
		 * Read and follow a chain of clusters
		 */
		spBuffer cFAT::clusterChainRead(size_t pCluster) {
			size_t totalbytes = directorySectors(pCluster) * blockSize();
			auto buffer = std::make_shared<tBuffer>();
			size_t remainbytes = totalbytes;

			while (remainbytes) {
				auto sector = clusterToBlock(pCluster);

				auto size = min(remainbytes, mBootBlock->mBiosParams.mSectorsPerCluster * blockSize());
				auto block = sourceBufferCopy(sector * blockSize(), size);
				if (!block)
					return 0;

				buffer->pushBuffer(block, size);
				remainbytes -= size;

				pCluster = fatSectorNext(pCluster);
				if (!pCluster && remainbytes) {
					if(warning("Cluster end reached, but remainbytes > 0")->isAborted())
						return {};

					totalbytes -= remainbytes;
					remainbytes = 0;
				}
			}
			buffer->resize(totalbytes);
			return buffer;
		}

		/**
		 * Load in a cluster of a directory listing
		 */
		bool cFAT::entrysLoad(fat::spDir pDir) {
			size_t cluster = pDir->mFirstCluster;

			spBuffer block;
			
			if(pDir == mFsRoot && mType != fat::eType_FAT32)
				block = clusterChainReadRoot(pDir->mBlock);
			else
				block = clusterChainRead(pDir->mFirstCluster);

			fat::sFileEntry* Entry = (fat::sFileEntry*) block->data();
			fat::sFileEntry* LastEntry = Entry + (block->size() / sizeof(fat::sFileEntry));

			std::vector<fat::sFileLongNameEntry*> LongEntries;

			for (; Entry != LastEntry; ++Entry) {
				if (!Entry->Name[0]) {
					break;
				}

				if (Entry->Name[0] == 0x20 || Entry->Name[0] > 0x80)
					continue;

				// Long File Name
				if (Entry->Attribute == 0x0F) {
					auto LongEntry = reinterpret_cast<fat::sFileLongNameEntry*>(Entry);
					LongEntries.push_back(LongEntry);
					continue;
				}

				auto entry = entryLoad(Entry, LongEntries);
				if (entry) {
					if (typeid(*entry) == typeid(fat::sDir)) {
						if (entry->nameGet() == "." || entry->nameGet() == "..")
							continue;
						entrysLoad(std::dynamic_pointer_cast<fat::sDir>(entry));
					}
					pDir->nodeAdd(entry);
				}
			}


			return true;
		}

		/**
		 * Load an individual entry in a directory listing
		 */
		spNode cFAT::entryLoad(const fat::sFileEntry* pEntry,  std::vector<fat::sFileLongNameEntry*>& pLongEntries) {
			spNode result;
			fat::sEntry* entry = 0;

			auto StartCluster = (uint32_t)pEntry->StartCluster;

			if (mType == fat::eType_FAT32) {
				StartCluster |= ((uint32_t)pEntry->StartClusterHi) << 16;
			}
			if (pEntry->Attributes.directory) {
				auto Dir = std::dynamic_pointer_cast<fat::sDir>(filesystemDirectoryCreate());
				result = Dir;
				entry = Dir.operator->();

				Dir->sizeInBytesSet(directorySectors(StartCluster) * blockSize());
			} else {
				auto File = std::dynamic_pointer_cast<fat::sFile>(filesystemFileCreate());
				result = File;
				entry = File.operator->();

				File->sizeInBytesSet(pEntry->FileLength);
			}

			entry->mFirstCluster = StartCluster;
			entry->mBlock = clusterToBlock(StartCluster);

			std::string name;

			name.append((const char*)pEntry->Name, 8);
			name = rtrim(name, 0x20);	// Trim spaces

			std::string extension = "";
			extension.append((const char*)pEntry->Extension, 3);
			extension = rtrim(extension, 0x20);	// Trim spaces

			if (!pEntry->Attributes.directory && !pEntry->Attributes.mLabel && extension.size()) {
				name.append(".");
			}

			name.append(extension);

			helpers::sDateTime date;
			date.days = (pEntry->Date & 0x1F);
			date.month = ((pEntry->Date & 0x1E0) >> 5);
			date.year = 1980 + ((pEntry->Date & 0xFE00) >> 9);
			date.secs = (pEntry->Time & 0x1F) * 2;
			date.mins = ((pEntry->Time & 0x7E0) >> 5);
			date.hour = ((pEntry->Time & 0xF800) >> 11);
			result->timeWriteSet(date);

			date.days = (pEntry->CrtDate & 0x1F);
			date.month = ((pEntry->CrtDate & 0x1E0) >> 5);
			date.year = 1980 + ((pEntry->CrtDate & 0xFE00) >> 9);
			date.secs = (pEntry->CrtTime & 0x1F) * 2;
			date.mins = ((pEntry->CrtTime & 0x7E0) >> 5);
			date.hour = ((pEntry->CrtTime & 0xF800) >> 11);
			result->timeCreateSet(date);
			

			// Is this the disk label?
			if (pEntry->Attributes.mLabel) {
				mLabel = name;
				return 0;;
			}

			result->nameSet(name);

			if (pLongEntries.size()) {
				entry->mShortName = result->nameGet();

				std::sort(pLongEntries.begin(), pLongEntries.end(), 
					[](const auto& lhs, const auto& rhs) {
						return (lhs->mSequence & 0x3F) < rhs->mSequence;
					});

				for (auto& lfn : pLongEntries) {
					entry->mUnicodeName.append((const wchar_t*)lfn->mName1, lstrlenW(lfn->mName1) > 5 ? 5 : lstrlenW(lfn->mName1));

					if ((lfn->mName2[0] == 0xFFFF) && (lfn->mName2[1] == 0xFFFF)) {
						break;
					}
					entry->mUnicodeName.append((const wchar_t*)lfn->mName2, lstrlenW(lfn->mName2) > 6 ? 6 : lstrlenW(lfn->mName2));

					if ((lfn->mName3[0] == 0xFFFF) && (lfn->mName3[1] == 0xFFFF)) {
						break;
					}
					entry->mUnicodeName.append((const wchar_t*)lfn->mName3, lstrlenW(lfn->mName3) > 2 ? 2 : lstrlenW(lfn->mName3));
				}
				
				auto size = entry->mUnicodeName.size();
				std::string mbstr;
				mbstr.resize(size + 1);
				wcstombs_s(&size, (char*)mbstr.data(), mbstr.size(), entry->mUnicodeName.data(), size);
				mbstr.resize(mbstr.size()-1);
				result->nameSet(mbstr);

				pLongEntries.clear();
			}
			result->dirty(false);
			return result;
		}

		/**
		 * Read a file from the filesystem
		 */
		spBuffer cFAT::filesystemRead(spNode pFile) {
			fat::spFile File = std::dynamic_pointer_cast<fat::sFile>(pFile);
			if (!File)
				return {};

			auto buffer = clusterChainRead(File->mFirstCluster);
			buffer->resize(File->sizeInBytesGet());
			return buffer;
		}

		bool cFAT::filesystemRemove(spNode pFile) {
			return false;
		}

		/**
		 *
		 */
		tBlock cFAT::blockCount() const {
			return (mBootBlock->mBiosParams.mSectorsTotal ? mBootBlock->mBiosParams.mSectorsTotal : mBootBlock->mBiosParams.mSectorsTotal_H);
		}

		/**
		 *
		 */
		size_t cFAT::blockSize(const tBlock pBlock) const {

			if (!mBootBlock)
				return cBlocks::blockSize(pBlock);

			return mBootBlock->mBiosParams.mBytesPerSector;
		}

		/**
		 *
		 */
		tBlock cFAT::blockToCluster(const tBlock pBlock) const {
			return ((pBlock - mBlockData) / mBootBlock->mBiosParams.mSectorsPerCluster) + 2;
		}

		/**
		 *
		 */
		bool cFAT::blockIsFree(const tBlock pBlock) const {
			return false;
		}

		/**
		 *
		 */
		bool cFAT::blockSet(const tBlock pBlock, const bool pValue) {
			return false;
		}

		/**
		 *
		 */
		std::vector<sAccessUnit> cFAT::blockUse(const tBlock pTotal) {
			return {};
		}

		/**
		 *
		 */
		bool cFAT::blocksFree(const std::vector<sAccessUnit>& pBlocks) {
			return false;
		}

		/**
		 * Return free clusters
		 */
		std::vector<sAccessUnit> cFAT::blocksGetFree() const {
			std::vector<sAccessUnit> free;
			tBlock clusternumber = 0;

			for (auto& cluster : mClusterMap) {
				if (!cluster) {
					auto block = clusterToBlock(clusternumber);
					for(auto current = block; current < block + mBootBlock->mBiosParams.mSectorsPerCluster; ++current)
						free.push_back( current );
				}
				++clusternumber;
			}
			return free;
		}

		size_t cFAT::filesystemTotalBytesFree() {
			return blocksGetFree().size() * blockSize();
		}

		size_t cFAT::filesystemTotalBytesMax() {
			return (mClustersTotal * mBootBlock->mBiosParams.mSectorsPerCluster) * blockSize();
		}

		bool cFAT::filesystemChainLoad(spFile pFile) {
			auto file = std::dynamic_pointer_cast<fat::sFile>(pFile);
			auto cluster = file->mFirstCluster;
			while (cluster) {
				auto sector = clusterToBlock(cluster);
				file->mChain.push_back({ sector });

				cluster = fatSectorNext(cluster);
				if (!cluster)
					break;
			}
			return true;
		}

		bool cFAT::filesystemBitmapLoad() {
			mClusterMap.clear();

			if (mType == fat::eType_FAT12) {
				auto blockFat = sourceBufferCopy(mBlockFAT * blockSize(), 4608 * 3);
				if (!blockFat)
					return false;

				for (size_t i = 0, j = 0; i < 4608; i += 3) {
					mClusterMap.push_back((blockFat->at(i) + (blockFat->at(i + 1) << 8)) & 0x0FFF);
					mClusterMap.push_back((blockFat->at(i + 1) + (blockFat->at(i + 2) << 8)) >> 4);
				}
				return true;
			}
			
			if (mType == fat::eType_FAT16) {
				size_t size = (mBootBlock->mBiosParams.mSectorsPerFAT * mBootBlock->mBiosParams.mFatCount) * blockSize();
				auto blockFat = sourceBufferCopy(mBlockFAT * blockSize(), size);
				if (!blockFat)
					return false;

				for (size_t i = 0; i < size; i += 2) {
					mClusterMap.push_back((blockFat->at(i) + (blockFat->at(i + 1) << 8)) & 0xFFFF);
				}
				return true;
			}

			if (mType == fat::eType_FAT32) {
				size_t size = (mBlockFAT + mBootBlock->mBiosParam32.mFatTotalSectors) * blockSize();
				auto blockFat = sourceBufferCopy(mBlockFAT * blockSize(), size);
				if (!blockFat)
					return false;

				for (size_t i = 0; i < size; i += 4) {
					uint32_t val = ((blockFat->at(i) |
						(blockFat->at(i + 1) << 8)) |
						(blockFat->at(i + 2) << 16) |
						(blockFat->at(i + 3) << 24));


					mClusterMap.push_back(val & 0x0fffffff);
				}
				return true;
			}

			return false;
		}

		bool cFAT::filesystemBitmapSave() {
			return false;
		}

	}
}
