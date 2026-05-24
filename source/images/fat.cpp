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
				mSizeInSectors = 0;
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
				0x15, //DOS 16-bit extended partition
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

		/**
		 * Test if this source is a FAT filesystem image
		 */
		bool cFAT::imageTest(spSource pSource) {
			if (!pSource)
				return false;

			const auto sourceSize = pSource->size();
			if (sourceSize < sizeof(fat::sBootRecordBlock))
				return false;

			auto validate = [&](const fat::sBootRecordBlock* pBootBlock, const size_t pOffsetBytes) -> bool {
				if (!pBootBlock)
					return false;
				if ((pBootBlock->mSignature1 != 0x55) || (pBootBlock->mSignature2 != 0xAA)) {
					if ((pBootBlock->mJmpBoot[0] != 0xEB) && (pBootBlock->mJmpBoot[0] != 0xE9))
						return false;
				}

				const auto bytesPerSector = readLEWord(&pBootBlock->mBiosParams.mBytesPerSector);
				if (!bytesPerSector || (bytesPerSector & (bytesPerSector - 1)))
					return false;
				if (bytesPerSector < 512 || bytesPerSector > 4096)
					return false;

				const auto sectorsPerCluster = pBootBlock->mBiosParams.mSectorsPerCluster;
				if (!sectorsPerCluster || (sectorsPerCluster & (sectorsPerCluster - 1)))
					return false;

				const auto sectorsPerFat = (size_t)(readLEWord(&pBootBlock->mBiosParams.mSectorsPerFAT) ? readLEWord(&pBootBlock->mBiosParams.mSectorsPerFAT) : readLEDWord(&pBootBlock->mBiosParam32.mFatTotalSectors));
				if (!sectorsPerFat)
					return false;

				const auto fatCount = pBootBlock->mBiosParams.mFatCount;
				if (!fatCount)
					return false;

				const auto sectorsReserved = readLEWord(&pBootBlock->mBiosParams.mSectorsReserved);
				if (!sectorsReserved)
					return false;

				const auto sectorsTotal = (uint32_t)(readLEWord(&pBootBlock->mBiosParams.mSectorsTotal) ? readLEWord(&pBootBlock->mBiosParams.mSectorsTotal) : readLEDWord(&pBootBlock->mBiosParams.mSectorsTotal_H));
				if (!sectorsTotal)
					return false;

				const uint64_t expectedBytes = (uint64_t)sectorsTotal * bytesPerSector;
				if (expectedBytes + pOffsetBytes > sourceSize)
					return false;

				const auto rootEntryCount = readLEWord(&pBootBlock->mBiosParams.mRootEntryCount);
				const auto rootDirSectors = rootEntryCount ? (((size_t)rootEntryCount * sizeof(fat::sFileEntry) + bytesPerSector - 1) / bytesPerSector) : 0;
				const auto fatSectors = (size_t)fatCount * sectorsPerFat;
				const auto reservedSectors = (size_t)sectorsReserved;
				if ((size_t)sectorsTotal <= (reservedSectors + fatSectors + rootDirSectors))
					return false;

				if (rootEntryCount == 0) {
					const auto rootCluster = readLEDWord(&pBootBlock->mBiosParam32.mRootCluster);
					if (rootCluster < 2)
						return false;
				}

				const auto dataSectors = sectorsTotal - (reservedSectors + fatSectors + rootDirSectors);
				const auto totalClusters = dataSectors / sectorsPerCluster;
				if (totalClusters < 2)
					return false;

				return true;
			};

			auto bootBlock = pSource->objectGet<fat::sBootRecordBlock>(0);
			if (!bootBlock)
				return false;

			if (validate(bootBlock.get(), 0))
				return true;

			// If this is a partitioned image, probe partition boot sectors.
			for (int part = 0; part < 4; ++part) {
				auto& entry = bootBlock->mMasterBootRecord.mPartitions[part];
				if (!fat::partitionTypeValid(entry.mType))
					continue;
				if (!entry.mStartLBA)
					continue;

				auto partitionOffset = (size_t)entry.mStartLBA * sizeof(fat::sBootRecordBlock);
				if (partitionOffset + sizeof(fat::sBootRecordBlock) > sourceSize)
					continue;
				auto partitionBoot = pSource->objectGet<fat::sBootRecordBlock>(partitionOffset);
				if (!partitionBoot)
					continue;

				if (validate(partitionBoot.get(), partitionOffset))
					return true;
			}

			return false;
		}

		bool partitionTypeIsExtended(uint8_t pType) {
			return (pType == 0x05 || pType == 0x0f || pType == 0x15);
		}

		bool partitionTypeIsFat(uint8_t pType) {
			if (!fat::partitionTypeValid(pType))
				return false;
			if (partitionTypeIsExtended(pType))
				return false;
			return true;
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
			if (pCluster < 2 || pCluster >= mClusterMap.size()) {
				//std::cout << "Invalid Cluster\n";
				return 0;
			}

			auto next = mClusterMap[pCluster] & 0x0FFFFFFF;
			if ((next == 0) || 
				(mType == fat::eType_FAT12 && next >= 0x0FF0 && next <= 0x0FFF) ||
				(mType == fat::eType_FAT16 && next >= 0xFFF0 && next <= 0xFFFF) ||
				(mType == fat::eType_FAT32 && next >= 0x0FFFFFF0 && next <= 0x0FFFFFFF))
				return 0;

			if (next == pCluster) {
				return 0;
			}
			
			return next;
		}

		tBlock cFAT::directorySectors(tBlock pStart) const {
			tBlock totalClusters = 1;
			auto maxClusters = (mClustersTotal ? mClustersTotal : (tBlock)mClusterMap.size());
			tBlock traversed = 0;

			while (pStart >= 2 && pStart < mClusterMap.size() && traversed < maxClusters) {
				pStart = fatSectorNext(pStart);
				traversed++;

				if (pStart)
					totalClusters++;
			}

			return totalClusters * mBootBlock->mBiosParams.mSectorsPerCluster;
		}

		/**
		 * Convert a cluster number to a block
		 */
		tBlock cFAT::clusterToBlock(tBlock pCluster) const {
			if (pCluster < 2)
				return mBlockData;
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

			std::vector<uint32_t> extendedPartitions;
			uint32_t fallback = 0;

			// Check the partition table
			for (int part = 0; part < 4; ++part) {
				auto& entry = bootBlock->mMasterBootRecord.mPartitions[part];
				if (entry.mType == 0)
					continue;
				if (!fat::partitionTypeValid(entry.mType))
					continue;

				if (fat::partitionTypeValid(entry.mType)) {
					if (partitionTypeIsFat(entry.mType)) {
						if (entry.mActive == 0x80 && entry.mStartLBA)
							return partitionOpen(entry.mStartLBA);
						if (!fallback)
							fallback = entry.mStartLBA;
					}
					else if (partitionTypeIsExtended(entry.mType) && entry.mStartLBA) {
						extendedPartitions.push_back(entry.mStartLBA);
					}
				}
			}

			for (auto base : extendedPartitions) {
				auto current = base;
				std::vector<uint32_t> visited;

				while (current) {
					auto it = std::find(visited.begin(), visited.end(), current);
					if (it != visited.end())
						break;
					visited.push_back(current);

					auto ebr = blockObjectGet<fat::sBootRecordBlock>(current);
					if (!ebr)
						break;

					uint32_t nextExtended = 0;
					for (int part = 0; part < 4; ++part) {
						auto& entry = ebr->mMasterBootRecord.mPartitions[part];
						if (entry.mType == 0)
							continue;
						if (!fat::partitionTypeValid(entry.mType))
							continue;
						if (partitionTypeIsFat(entry.mType) && entry.mStartLBA) {
							return partitionOpen(current + entry.mStartLBA);
						}

						if (partitionTypeIsExtended(entry.mType) && entry.mStartLBA && !nextExtended) {
							nextExtended = base + entry.mStartLBA;
						}
					}

					current = nextExtended;
				}
			}

			if (fallback) {
				return partitionOpen(fallback);
			}

			// Fallback: superfloppy style disk
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
			const auto sectorsPerFat = (size_t)(mBootBlock->mBiosParams.mFatCount ? mBootBlock->mBiosParams.mSectorsPerFAT : mBootBlock->mBiosParam32.mFatTotalSectors);
			if (sectorsPerFat && mBootBlock->mBiosParams.mSectorsPerCluster <= 128) {
				if (mBootBlock->mBiosParams.mRootEntryCount) {
					mBlockRoot = (mBlockFAT + (sectorsPerFat * mBootBlock->mBiosParams.mFatCount));
					mBlockData = mBlockRoot + (((mBootBlock->mBiosParams.mRootEntryCount * sizeof(fat::sFileEntry)) + (blockSize() - 1)) / blockSize());
					mClusterRoot = 0;
				}
			} else {
				if (mBootBlock->mBiosParam32.mFatTotalSectors && mBootBlock->mBiosParams.mFatCount) {
					mBlockData = mBlockFAT + (mBootBlock->mBiosParam32.mFatTotalSectors * mBootBlock->mBiosParams.mFatCount);
					mClusterRoot = mBootBlock->mBiosParam32.mRootCluster;
					mBlockRoot = clusterToBlock(mClusterRoot);
				}
			}

			if (mBootBlock->mBiosParams.mSectorsPerCluster) {
				mClustersTotal = (blockCount() - mBlockData) / mBootBlock->mBiosParams.mSectorsPerCluster;
				if (mClustersTotal < 4085)
					mType = fat::eType_FAT12;
				else if (mClustersTotal < 65525)
					mType = fat::eType_FAT16;
				else if (mClustersTotal <= 268435455)
					mType = fat::eType_FAT32;
			}

			if (mType == fat::eType_Unknown || !filesystemBitmapLoad()) {
				return false;
			}

			auto Root = std::make_shared<fat::sDir>(weak_from_this());
			Root->mFirstCluster = mClusterRoot;
			Root->mBlock = mBlockRoot;
			Root->sizeInBytesSet(mBootBlock->mBiosParams.mRootEntryCount * sizeof(fat::sFileEntry));

			if (!Root->sizeInBytesGet()) {
				Root->sizeInBytesSet(directorySectors(blockToCluster(Root->mBlock)) * blockSize());
			}
			mFsRoot = Root;
			Root->entriesLoadedSet(false);
			return true;
		}

		bool cFAT::filesystemDirectoryLoad(spDirectory pDir) {
			auto dir = std::dynamic_pointer_cast<fat::sDir>(pDir);
			if (!dir)
				return false;

			return entriesLoad(dir);
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
		bool cFAT::entriesLoad(fat::spDir pDir) {
			pDir->mNodes.clear();
			pDir->entriesLoadedSet(false);

			spBuffer block;
			
			if(pDir == mFsRoot && mType != fat::eType_FAT32)
				block = clusterChainReadRoot(pDir->mBlock);
			else
				block = clusterChainRead(pDir->mFirstCluster);

			if (!block)
				return false;

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
					if (typeid(*entry) == typeid(fat::sDir) && (entry->nameGet() == "." || entry->nameGet() == ".."))
						continue;
					pDir->nodeAdd(entry);
				}
			}

			pDir->entriesLoadedSet(true);

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
			if (!buffer)
				return {};

			if (File->sizeInBytesGet() > buffer->size())
				return {};

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
			if (!mBootBlock)
				return false;

			if (pBlock < mBlockData)
				return false;

			auto cluster = blockToCluster(pBlock);
			if (cluster < 2 || cluster >= mClustersTotal + 2)
				return false;
			if (cluster >= mClusterMap.size())
				return false;

			return mClusterMap[cluster] == 0;
		}

		/**
		 *
		 */
		bool cFAT::blockSet(const tBlock pBlock, const bool pValue) {
			if (!mBootBlock)
				return false;

			if (pBlock < mBlockData)
				return false;

			auto cluster = blockToCluster(pBlock);
			if (cluster < 2 || cluster >= mClusterMap.size())
				return false;
			if (cluster >= mClustersTotal + 2)
				return false;

			dirty();

			if (!pValue) {
				mClusterMap[cluster] = 0;
				return true;
			}

			if (mType == fat::eType_FAT12)
				mClusterMap[cluster] = 0x0FFF;
			else if (mType == fat::eType_FAT16)
				mClusterMap[cluster] = 0xFFFF;
			else
				mClusterMap[cluster] = 0x0FFFFFFF;

			return true;
		}

		/**
		 *
		 */
		std::vector<sAccessUnit> cFAT::blockUse(const tBlock pTotal) {
			std::vector<sAccessUnit> results;
			if (!pTotal || !mBootBlock)
				return results;

			for (tBlock cluster = 2; cluster < mClusterMap.size() && cluster < (mClustersTotal + 2); ++cluster) {
				auto block = clusterToBlock(cluster);
				if (blockIsFree(block)) {
					results.push_back({ block });

					if (results.size() == pTotal) {
						for (size_t index = 0; index < results.size(); ++index) {
							if (!blockSet(results[index].block(), true)) {
								blocksFree(results);
								return {};
							}
						}
						return results;
					}
				}
			}

			error("Not enough free blocks");
			return {};
		}

		/**
		 *
		 */
		bool cFAT::blocksFree(const std::vector<sAccessUnit>& pBlocks) {
			for (auto& block: pBlocks) {
				if (!blockSet(block.block(), false)) {
					error(" blocksFree-blockSet failed");
					return false;
				}
			}

			return true;
		}

		/**
		 * Return free clusters
		 */
		std::vector<sAccessUnit> cFAT::blocksGetFree() const {
			std::vector<sAccessUnit> free;
			auto end = ((size_t)(mClustersTotal + 2) < mClusterMap.size()) ? (size_t)(mClustersTotal + 2) : mClusterMap.size();
			for (tBlock cluster = 2; cluster < end; ++cluster) {
				if (mClusterMap[cluster] == 0) {
					free.push_back(clusterToBlock(cluster));
				}
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
				if (cluster < 2 || cluster >= mClusterMap.size()) {
					return false;
				}
			}
			return true;
		}

		bool cFAT::filesystemBitmapLoad() {
			mClusterMap.clear();

			if (mType == fat::eType_FAT12) {
				size_t sectorsPerFat = (size_t)(mBootBlock->mBiosParams.mSectorsPerFAT ? mBootBlock->mBiosParams.mSectorsPerFAT : mBootBlock->mBiosParam32.mFatTotalSectors);
				size_t size = sectorsPerFat * blockSize();
				auto blockFat = sourceBufferCopy(mBlockFAT * blockSize(), size);
				if (!blockFat)
					return false;

				for (size_t i = 0; i + 2 < size; i += 3) {
					mClusterMap.push_back((blockFat->at(i) | (blockFat->at(i + 1) << 8)) & 0x0FFF);
					mClusterMap.push_back(((blockFat->at(i + 1) >> 4) | (blockFat->at(i + 2) << 4)) & 0x0FFF);
				}
			} else if (mType == fat::eType_FAT16) {
				size_t sectorsPerFat = (size_t)(mBootBlock->mBiosParams.mSectorsPerFAT ? mBootBlock->mBiosParams.mSectorsPerFAT : mBootBlock->mBiosParam32.mFatTotalSectors);
				size_t size = sectorsPerFat * blockSize();
				auto blockFat = sourceBufferCopy(mBlockFAT * blockSize(), size);
				if (!blockFat)
					return false;

				for (size_t i = 0; i + 1 < size; i += 2) {
					mClusterMap.push_back((blockFat->at(i) + (blockFat->at(i + 1) << 8)) & 0xFFFF);
				}
			} else if (mType == fat::eType_FAT32) {
				size_t sectorsPerFat = mBootBlock->mBiosParam32.mFatTotalSectors;
				if (!sectorsPerFat)
					sectorsPerFat = mBootBlock->mBiosParams.mSectorsPerFAT;
				size_t size = sectorsPerFat * blockSize();
				auto blockFat = sourceBufferCopy(mBlockFAT * blockSize(), size);
				if (!blockFat)
					return false;

				for (size_t i = 0; i + 3 < size; i += 4) {
					uint32_t val = ((blockFat->at(i) |
						(blockFat->at(i + 1) << 8)) |
						(blockFat->at(i + 2) << 16) |
						(blockFat->at(i + 3) << 24));

					mClusterMap.push_back(val & 0x0fffffff);
				}
			} else {
				return false;
			}

			if (!mClusterMap.size())
				return false;

			auto clustersExpected = mClustersTotal + 2;
			if (clustersExpected && mClusterMap.size() > clustersExpected) {
				mClusterMap.resize(clustersExpected);
			}

			if (clustersExpected && mClusterMap.size() < clustersExpected)
				return false;

			return true;
		}

		bool cFAT::filesystemBitmapSave() {
			if (!mBootBlock || mClusterMap.empty() || !mBootBlock->mBiosParams.mFatCount)
				return false;

			size_t sectorsPerFat = 0;
			if (mType == fat::eType_FAT12 || mType == fat::eType_FAT16)
				sectorsPerFat = (size_t)(mBootBlock->mBiosParams.mSectorsPerFAT ? mBootBlock->mBiosParams.mSectorsPerFAT : mBootBlock->mBiosParam32.mFatTotalSectors);
			else if (mType == fat::eType_FAT32) {
				sectorsPerFat = mBootBlock->mBiosParam32.mFatTotalSectors;
				if (!sectorsPerFat)
					sectorsPerFat = mBootBlock->mBiosParams.mSectorsPerFAT;
			}

			if (!sectorsPerFat)
				return false;

			auto sectorBytes = blockSize();
			auto fatBytes = sectorsPerFat * sectorBytes;
			if (!fatBytes)
				return false;

			auto clusterCount = mClustersTotal + 2;
			if (clusterCount > mClusterMap.size())
				return false;

			auto buffer = std::make_shared<tBuffer>();
			buffer->resize(fatBytes);
			for (size_t index = 0; index < fatBytes; ++index) {
				buffer->at(index) = 0;
			}

			if (mType == fat::eType_FAT12) {
				size_t inIndex = 0;
				for (size_t outOffset = 0; outOffset + 2 < fatBytes && inIndex + 1 < clusterCount; outOffset += 3) {
					uint16_t first = mClusterMap[inIndex++] & 0x0FFF;
					uint16_t second = (inIndex < clusterCount) ? (mClusterMap[inIndex++] & 0x0FFF) : 0;
					buffer->at(outOffset) = static_cast<uint8_t>(first & 0xFF);
					buffer->at(outOffset + 1) = static_cast<uint8_t>((first >> 8) | ((second & 0x0F) << 4));
					buffer->at(outOffset + 2) = static_cast<uint8_t>(second >> 4);
				}
			}

			if (mType == fat::eType_FAT16) {
				for (size_t inIndex = 0, outOffset = 0; outOffset + 1 < fatBytes && inIndex < clusterCount; ++inIndex, outOffset += 2) {
					uint16_t value = mClusterMap[inIndex] & 0xFFFF;
					buffer->at(outOffset) = static_cast<uint8_t>(value);
					buffer->at(outOffset + 1) = static_cast<uint8_t>(value >> 8);
				}
			}

			if (mType == fat::eType_FAT32) {
				for (size_t inIndex = 0, outOffset = 0; outOffset + 3 < fatBytes && inIndex < clusterCount; ++inIndex, outOffset += 4) {
					uint32_t value = mClusterMap[inIndex] & 0x0FFFFFFF;
					buffer->at(outOffset) = static_cast<uint8_t>(value);
					buffer->at(outOffset + 1) = static_cast<uint8_t>(value >> 8);
					buffer->at(outOffset + 2) = static_cast<uint8_t>(value >> 16);
					buffer->at(outOffset + 3) = static_cast<uint8_t>(value >> 24);
				}
			}

			for (uint8_t fat = 0; fat < mBootBlock->mBiosParams.mFatCount; ++fat) {
				auto offset = (mBlockFAT + (size_t)fat * sectorsPerFat) * sectorBytes;
				if (!sourceBufferWrite(offset, buffer))
					return false;
			}

			return true;
		}

	}
}
