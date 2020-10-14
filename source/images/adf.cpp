#include "firy.hpp"
#include "adf.hpp"
#include <time.h>

namespace firy {
	namespace images {

		namespace adf {

			/**
			 * Maximum number of bytes which can be stored in a block
			 */
			const size_t gBytesPerBlock = 512;

			// Thanks to adflib for endian/time functions
			// http://lclevy.free.fr/adflib

			const int SW_LONG = 4;
			const int SW_SHORT = 2;
			const int SW_CHAR = 1;

			const int ST_ROOT = 1;
			const int ST_DIR = 2;

			/**
			 * Table of values which require endian flipping
			 */
			int blockEndianSwapTable[][15] = {
				{ 4, SW_CHAR, 2, SW_LONG, 1012, SW_CHAR, 0, 1024 },				/* first bytes of boot */
				{ 108, SW_LONG, 40, SW_CHAR, 10, SW_LONG, 0, 512 },				/* root */
				{ 6, SW_LONG, 488, SW_CHAR, 0, 512 },							/* data */
																				/* file, dir, entry */
				{ 82, SW_LONG, 92, SW_CHAR, 3, SW_LONG, 36, SW_CHAR, 11, SW_LONG, 0, 512 },
				{ 6, SW_LONG, 0, 24 },                                       /* cache */
				{ 128, SW_LONG, 0, 512 },									/* bitmap, fext */
																			/* link */
				{ 6, SW_LONG, 64, SW_CHAR, 86, SW_LONG, 32, SW_CHAR, 12, SW_LONG, 0, 512 },
				{ 4, SW_CHAR, 39, SW_LONG, 56, SW_CHAR, 10, SW_LONG, 0, 256 }, /* RDSK */
				{ 4, SW_CHAR, 127, SW_LONG, 0, 512 },                          /* BADB */
				{ 4, SW_CHAR, 8, SW_LONG, 32, SW_CHAR, 31, SW_LONG, 4, SW_CHAR, /* PART */
				  15, SW_LONG, 0, 256 },
				{ 4, SW_CHAR, 7, SW_LONG, 4, SW_CHAR, 55, SW_LONG, 0, 256 }, /* FSHD */
				{ 4, SW_CHAR, 4, SW_LONG, 492, SW_CHAR, 0, 512 }             /* LSEG */
			};

			static uint32_t blockBamBitMask[32] = {
				0x1, 0x2, 0x4, 0x8,
				0x10, 0x20, 0x40, 0x80,
				0x100, 0x200, 0x400, 0x800,
				0x1000, 0x2000, 0x4000, 0x8000,
				0x10000, 0x20000, 0x40000, 0x80000,
				0x100000, 0x200000, 0x400000, 0x800000,
				0x1000000, 0x2000000, 0x4000000, 0x8000000,
				0x10000000, 0x20000000, 0x40000000, 0x80000000
			};

			/**
			 * is leap year
			 */
			bool isLeapYear(int pYear) {
				return ((bool) (!(pYear % 100) ? !(pYear % 400) : !(pYear % 4)));
			}

			/**
			 * Default datetime to now
			 */
			sDateTime::sDateTime() {
				struct tm local;
				time_t cal;

				time(&cal);
				localtime_s(&local, &cal);

				year = local.tm_year;         /* since 1900 */
				month = local.tm_mon + 1;
				days = local.tm_mday;
				hour = local.tm_hour;
				mins = local.tm_min;
				secs = local.tm_sec;
			}

			/**
			 *
			 */
			void convertDaysToDate(int32_t days, int* yy, int* mm, int* dd) {
				int y, m;
				int nd;
				int jm[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

				/* 0 = 1 Jan 1978,  6988 = 18 feb 1997 */

				/*--- year ---*/
				y = 1978;
				if (isLeapYear(y))
					nd = 366;
				else
					nd = 365;
				while (days >= nd) {
					days -= nd;
					y++;
					if (isLeapYear(y))
						nd = 366;
					else
						nd = 365;
				}

				/*--- month ---*/
				m = 1;
				if (isLeapYear(y))
					jm[2 - 1] = 29;
				while (days >= jm[m - 1]) {
					days -= jm[m - 1];
					m++;
				}

				*yy = y;
				*mm = m;
				*dd = days + 1;
			}

			void convertTimeToAmigaTime(struct sDateTime dt, int32_t* day, int32_t* min, int32_t* ticks)
			{
				int jm[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

				*min = dt.hour * 60 + dt.mins;                /* mins */
				*ticks = dt.secs * 50;                        /* ticks */

				/*--- days ---*/

				*day = dt.days - 1;                         /* current month days */

				/* previous months days downto january */
				if (dt.month > 1) {                      /* if previous month exists */
					dt.month--;
					if (dt.month > 2 && isLeapYear(dt.year))    /* months after a leap february */
						jm[2 - 1] = 29;
					while (dt.month > 0) {
						*day = *day + jm[dt.month - 1];
						dt.month--;
					}
				}

				/* years days before current year downto 1978 */
				if (dt.year > 78) {
					dt.year--;
					while (dt.year >= 78) {
						if (isLeapYear(dt.year))
							* day = *day + 366;
						else
							*day = *day + 365;
						dt.year--;
					}
				}
			}

			/*
			 * blockSwapEndian
			 *
			 * magic :-) endian swap function (big -> little for read, little to big for write)
			 */
			void blockSwapEndian(uint8_t* buf, int mType) {
				int i = 0, j = 0;
				int p = 0;

				while (blockEndianSwapTable[mType][i] != 0) {
					for (j = 0; j < blockEndianSwapTable[mType][i]; j++) {
						switch (blockEndianSwapTable[mType][i + 1]) {
						case SW_LONG:

							*(uint32_t*)(buf + p) = readBEDWord(buf + p);
							p += 4;
							break;
						case SW_SHORT:
							*(uint32_t*)(buf + p) = readBEWord(buf + p);
							p += 2;
							break;
						case SW_CHAR:
							p++;
							break;
						default:
							;
						}
					}
					i += 2;
				}

				if (p != blockEndianSwapTable[mType][i + 1]) {
					throw std::exception("blockSwapEndian length");
				}
			}

			/**
			 * File inside an ADF
			 */
			sFile::sFile(wpFilesystem pFilesystem, const std::string& pName) : sEntry(), filesystem::sFile(pFilesystem, pName) {

			}

			/**
			 * Directory inside ADF
			 */
			sDir::sDir(wpFilesystem pFilesystem, const std::string& pName) : sEntry(), filesystem::sDirectory(pFilesystem, pName) {

			}
		}

		/**
		 * Constructor
		 */
		cADF::cADF(spSource pSource) : cImageAccess<access::cBlocks>(), access::cInterface(pSource) {
			mBlockSize = adf::gBytesPerBlock;

			mBlockFirst = 0;
			mBlockLast = 0;
			mBlockRoot = 0;

			mBlockCount = pSource->size() / mBlockSize;
		}

		/**
		 * Generate the checksum for a bootblock
		 */
		uint32_t cADF::blockBootChecksum(const uint8_t* pBuffer, const size_t pBufferLen) {
			int32_t d, newSum = 0;
			for (size_t i = 0; i < 256; i++) {
				if (i != 1) {
					d = readBEDWord(pBuffer + i * 4);
					if ((0xffffffffU - newSum) < (uint32_t)d)
						newSum++;
					newSum += d;
				}
			}
			return(~newSum);
		}

		/**
		 * Generate the checksum for a regular block, skipping the checksum field
		 */
		uint32_t cADF::blockChecksum(const uint8_t* pBuffer, const size_t pBufferLen, const size_t pChecksumByte) {
			int32_t newsum = 0;
			for (size_t i = 0; i < (pBufferLen / 4); i++)
				if (i != (pChecksumByte / 4))	// Skip checksum
					newsum += readBEDWord(pBuffer + i * 4);

			return -newsum;
		}

		/**
		 * Load the filesystem metadata
		 */
		bool cADF::filesystemLoad() {
			if (!blockBootLoad())
				return false;
			if (!blockRootLoad())
				return false;

			if (!filesystemBitmapLoad())
				return false;

			auto Root = std::make_shared<adf::sDir>(weak_from_this(), "/");
			Root->mBlock = mBlockRoot;

			mFsRoot = Root;
			return entrysLoad(Root);
		}

		/**
		 *
		 */
		bool cADF::filesystemSave() {
			if (!filesystemSaveNode(mFsRoot, 0))
				return false;

			if (!filesystemBitmapSave())
				return false;

			dirty(false);
			return true;;
		}

		/**
		 *
		 */
		bool cADF::filesystemSaveNode(spNode pNode, adf::spDir pParent) {
			auto entry = std::dynamic_pointer_cast<adf::sEntry>(pNode);
			auto file = std::dynamic_pointer_cast<adf::sFile>(pNode);
			if (file) {
				if (file->isDirty()) {
					file->mBlock = entryGet(pParent, pNode);
					if (file->mBlock == -1 || file->mBlock == -2) {
						return false;
					}

					auto block = blockObjectCreate<adf::sFileHeaderBlock>();
					block->nameLen = (char)min(adf::MAXNAMELEN, file->nameGet().size());
					memcpy(block->fileName, file->nameGet().data(), block->nameLen);

					block->headerKey = file->mBlock;
					block->mType = 2;	//T_HEADER
					block->dataSize = 0;
					block->secType = -3;
					block->parent = (int32_t) pParent->mBlock;
					block->byteSize = (uint32_t) file->mContent->size();

					// Loop until EOF
					for (int index = adf::MAX_DATABLK - 1; index >= 0; --index) {
						auto nextBlock = blockUseSingle();

						if(!block->firstData)
							block->firstData = nextBlock;

						block->dataBlocks[index] = (int32_t) nextBlock;
						++block->highSeq;

						spBuffer buffer = std::make_shared<tBuffer>();
						buffer->pushBuffer(file->mContent->takeBytes(adf::gBytesPerBlock < file->mContent->size() ? adf::gBytesPerBlock : file->mContent->size()));
						blockWrite(nextBlock, buffer);

						if (!file->mContent->size())
							break;
					}
					// Need extension?
					tBlock ext = 0;
					if (file->mContent->size())
						ext = blockUseSingle();
					block->extension = ext;

					blockSave(file->mBlock, block);

					while (file->mContent->size()) {
						auto blockExt = blockObjectCreate<adf::sFileExtBlock>();
						blockExt->headerKey = ext;

						for (int index = adf::MAX_DATABLK - 1; index >= 0; --index) {
							auto nextBlock = blockUseSingle();

							blockExt->dataBlocks[index] = (int32_t)nextBlock;

							spBuffer buffer = std::make_shared<tBuffer>();
							buffer->pushBuffer(file->mContent->takeBytes(adf::gBytesPerBlock < file->mContent->size() ? adf::gBytesPerBlock : file->mContent->size()));
							blockWrite(nextBlock, buffer);

							if (!file->mContent->size())
								break;
						}

						tBlock extNxt = 0;
						if (file->mContent->size())
							extNxt = blockUseSingle();
						blockExt->extension = extNxt;

						blockSave(ext, blockExt);
						ext = extNxt;
					}
				}
				return true;
			}

			auto dir = std::dynamic_pointer_cast<adf::sDir>(pNode);
			if (dir) {
				if (dir->isDirty() && pParent) {
					auto newBlock = entryGet(pParent, pNode);
					if (newBlock == -1) {
						return false;
					}
					std::shared_ptr<adf::sFileHeaderBlock> block;

					if (newBlock >= 0) {
						block = blockObjectCreate<adf::sFileHeaderBlock>();
						block->nameLen = (char) min(adf::MAXNAMELEN, file->nameGet().size());
						memcpy(block->fileName, file->nameGet().data(), block->nameLen);

						block->headerKey = newBlock;
						block->parent = (int32_t)pParent->mBlock;

						block->highSeq = 0;
						block->dataSize = 0;	// hashTablesize
						block->mType = 2;	//T_HEADER
						block->secType = 2;
					} else {
						block = blockObjectGet<adf::sFileHeaderBlock>(entry->mBlock);
					}

					blockSave(block->headerKey, block);
				}

				// Sub Dirs
				for (auto node : dir->mNodes) {
					if (!filesystemSaveNode(node, dir))
						return false;
				}
			}

			return true;
		}

		/**
		 * Get the sector for a hash from this directory
		 */
		int32_t cADF::entryGet(adf::spDir pDir, spNode pNode) {
			auto block = blockLoad<adf::sEntryBlock>(pDir->mBlock);
			if (!block)
				return -1;

			auto entry = std::dynamic_pointer_cast<adf::sEntry>(pNode);
			auto hash = entry->getNameHash(false);

			auto newSect = (int32_t)blockUseSingle();

			// Does our hash exist already
			auto sect = block->hashTable[hash];
			if (sect == 0) {

				block->hashTable[hash] = newSect;
				adf::convertTimeToAmigaTime({}, &(block->days), &(block->mins), &(block->ticks));
				if (!blockSave(pDir->mBlock, block)) {
					blockSet(newSect, false);
					return -1;
				}

				return newSect;
			}

			// Hash already exists
			// Follow chain
			std::shared_ptr<adf::sEntryBlock> nextBlock;

			while (sect) {
				nextBlock = blockLoad<adf::sEntryBlock>(sect);
				if (!nextBlock) {
					gDebug->error("adf: next block not found");
					return -1;
				}

				// Check if its the file we are trtying to save
				if (nextBlock->nameLen == pNode->nameGet().size()) {
					auto name = std::string(nextBlock->name, min(nextBlock->nameLen, adf::MAXNAMELEN));

					if (pNode->nameGet() == name) {
						return -2;
					}
				}

				sect = nextBlock->nextSameHash;
			}

			// Update the block to reference the new sector
			nextBlock->nextSameHash = newSect;
			if (!blockSave(nextBlock->headerKey, nextBlock)) {
				gDebug->error("adf: block update failed");
				return -1;
			}

			return newSect;
		}

		/**
		 * Load the contents of a directory
		 */
		bool cADF::entrysLoad(adf::spDir pDir) {
			auto block = blockLoad<adf::sEntryBlock>(pDir->mBlock);
			if (!block)
				return false;

			for (auto& hash : block->hashTable) {
				if (hash) {
					auto entry = entryLoad(hash);
					if (!entry) {
						// TODO: Error
						continue;
					}
					pDir->mNodes.push_back(entry);

					// Load any further files with a matching hash
					tBlock nextSector = std::dynamic_pointer_cast<adf::sEntry>(entry)->mNextSameHash;
					while (nextSector) {
						auto entry = entryLoad(nextSector);
						if (!entry) {
							// TODO: Error
							break;
						}

						pDir->mNodes.push_back(entry);
						nextSector = std::dynamic_pointer_cast<adf::sEntry>(entry)->mNextSameHash;
					}
				}
			}
			return true;
		}

		/**
		 * Load an entry
		 */
		spNode cADF::entryLoad(const tBlock pBlock) {
			auto blockEntry = blockLoad<adf::sEntryBlock>(pBlock);
			if (!blockEntry) {
				// TODO: Record error
				return 0;
			}

			spNode node;
			std::shared_ptr<adf::sEntry> entry;

			switch (blockEntry->secType) {
			case 1:		// ST_ROOT
				return 0;
			case 2: 	// ST_DIR
			case 4:	{	// ST_LDIR
				node = std::make_shared<adf::sDir>(weak_from_this());
				entry = std::dynamic_pointer_cast<adf::sEntry>(node);
				entry->mReal = blockEntry->realEntry;
				break;
			}

			case 3:		// ST_LSOFT
			case -3: 	// ST_FILE
			case -4: {	// ST_LFILE
				node = filesystemFileCreate();
				entry = std::dynamic_pointer_cast<adf::sEntry>(node);
				entry->mReal = blockEntry->realEntry;
				break;
			}
			
			default:
				gDebug->error("adf: Unknown entry type: ", std::to_string(blockEntry->secType));
				return 0;	// TODO
				break;
			}

			if (!node || !entry)
				return 0;

			entry->mType = blockEntry->secType;
			entry->mParent = blockEntry->parent;

			node->nameSet(std::string(blockEntry->name, min(blockEntry->nameLen, adf::MAXNAMELEN)));

			adf::convertDaysToDate(blockEntry->days, &(entry->mDate.year), &(entry->mDate.month), &(entry->mDate.days));
			entry->mDate.hour = blockEntry->mins / 60;
			entry->mDate.mins = blockEntry->mins % 60;
			entry->mDate.secs = blockEntry->ticks / 50;

			entry->mBlock = pBlock;
			entry->mNextSameHash = blockEntry->nextSameHash;

			if(blockEntry->secType == 2)	// ST_DIR
				entrysLoad(std::dynamic_pointer_cast<adf::sDir>(node));

			node->dirty(false);
			return node;
		}

		/**
		 * 
		 */
		spBuffer cADF::filesystemRead(spNode pFile) {
			adf::spFile File = std::dynamic_pointer_cast<adf::sFile>(pFile);
			if (!File)
				return {};

			auto blockFile = blockLoad<adf::sFileHeaderBlock>(File->mBlock);
			size_t totalbytes = blockFile->byteSize;

			spBuffer buffer = std::make_shared<tBuffer>();

			// Old 1.2 Filesystem
			if (!(mBootBlock->dosType[3] & adf::eFlags::FFS)) {
				tBlock currentblock = blockFile->firstData;
				int count = 1;
				size_t offset = 0;

				while (currentblock) {
					auto block = blockLoad<adf::sOFSDataBlock>(currentblock);
					buffer->pushBuffer(block->data, block->dataSize);

					totalbytes -= block->dataSize;
					if (!totalbytes)
						return buffer;

					if (block->seqNum + 1 != ++count) {
						gDebug->error("adf", "Old FS: Sequence is wrong");
						return {};
					}
					currentblock = block->nextData;
				}

			} else {
				// New Filesystem
				for (int index = adf::MAX_DATABLK-1; index >= 0; --index) {
					if (!blockFile->dataBlocks[index])
						break;
					
					auto block = blockRead(blockFile->dataBlocks[index]);
					auto size = min(totalbytes, blockSize());
					block->resize(size);
					buffer->pushBuffer(block);
					totalbytes -= size;
				}

				tBlock nextSector = blockFile->extension;
				while (nextSector) {
					auto blockExt = blockLoad<adf::sFileExtBlock>(nextSector);
					if (!blockExt) {
						gDebug->error("adf", "sFileExtBlock not found");
						break;
					}

					for (int index = adf::MAX_DATABLK - 1; index >= 0; --index) {
						if (!blockExt->dataBlocks[index])
							break;

						auto block = blockRead(blockExt->dataBlocks[index]);
						auto size = min(totalbytes, blockSize());
						block->resize(size);
						buffer->pushBuffer(block);
						totalbytes -= size;
					}

					nextSector = blockExt->extension;
				}
			}

			buffer->resize(blockFile->byteSize);
			return buffer;
		}
		
		/**
		 *
		 */
		bool cADF::filesystemRemove(spNode pFile) {
			return false;
		}

		/**
		 * Size of a block
		 */
		size_t cADF::blockSize(const tBlock pBlock) const {
			return adf::gBytesPerBlock;
		}

		/** 
		 * Is a block free
		 */
		bool cADF::blockIsFree(const tBlock pBlock) const {
			tBlock sectOfMap = pBlock - 2;
			tBlock block = sectOfMap / (127 * 32);
			tBlock indexInMap = (sectOfMap / 32) % 127;

			return ((mBitmapBlocks[block]->map[indexInMap]
				& adf::blockBamBitMask[sectOfMap % 32]) != 0);
		}

		/**
		 * Set a block used
		 */
		bool cADF::blockSet(const tBlock pBlock, const bool pValue) {
			tBlock sectOfMap = pBlock - 2;
			tBlock block = sectOfMap / (127 * 32);
			tBlock indexInMap = (sectOfMap / 32) % 127;

			if (block >= mBitmapBlocks.size())
				return false;

			dirty();
			auto& map = mBitmapBlocks[block]->map[indexInMap];
			auto bit = adf::blockBamBitMask[sectOfMap % 32];

			if (pValue) {
				map &= ~bit;
				return true;
			}

			map |= bit;
			return true;
		}

		/**
		 * Get a number of blocks, marking them used in the process
		 */
		std::vector<tBlock> cADF::blockUse(const tBlock pTotal) {
			std::vector<tBlock> results;

			for (tBlock block = 2; block < blockCount(); ++block) {
				
				if (blockIsFree(block)) {
					results.push_back(block);

					if (results.size() == pTotal) {

						for (auto block : results) {
							if (!blockSet(block, true)) {
								gDebug->error("adf: blockUse-blockSet failed");
								return {};
							}
						}
						return results;
					}
				}
			}

			return {};
		}

		/**
		 *
		 */
		bool cADF::blocksFree(const std::vector<tBlock>& pBlocks) {
			for (auto& block: pBlocks) {
				if (!blockSet(block, false)) {
					gDebug->error("adf: blocksFree-blockSet failed");
					return false;
				}
			}

			return true;
		}

		/**
		 * Get all blocks free
		 */
		std::vector<tBlock> cADF::blocksGetFree() const {
			std::vector<tBlock> freeBlocks;
			for (tBlock j = mBlockFirst + 2; j <= (mBlockLast - mBlockFirst); j++)
				if (blockIsFree(j))
					freeBlocks.push_back(j);

			return freeBlocks;
		}

		/**
		 * Type of disk represented
		 */
		adf::eType cADF::diskType() const {
			if ((sourceSize() >= 512 * 11 * 2 * 80) &&
				(sourceSize() <= 512 * 11 * 2 * 83))
				return(adf::eType::FLOPPY_DD);

			else if (sourceSize() == 512 * 22 * 2 * 80)
				return(adf::eType::FLOPPY_HD);

			else if (sourceSize() > 512 * 22 * 2 * 80)
				return(adf::eType::HARDDRIVE);
			
			return adf::eType::eType_Unknown;
		}

		/**
		 * Name of the disk
		 */
		std::string cADF::filesystemNameGet() const {
			return std::string(mRootBlock->diskName, mRootBlock->nameLen);
		}

		/**
		 *
		 */
		bool cADF::filesystemChainLoad(spFile pFile) {
			return false;
		}
		
		/**
		 * Load the disk bitmap
		 */
		bool cADF::filesystemBitmapLoad() {
			mBitmapBlocks.clear();

			for (auto& page : mRootBlock->bmPages) {
				if (page) {
					auto block = blockLoad<adf::sBitmapBlock>(page);
					if (!block) {
						gDebug->error("bitmap block load error");
						return false;
					}
					mBitmapBlocks.push_back(block);
				}
			}

			auto blockBitmapExt = mRootBlock->bmExt;
			while (blockBitmapExt) {
				auto blockExt = blockLoadNoCheck<adf::sBitmapExtBlock>(blockBitmapExt);
				for(auto& page : blockExt->bmPages) {
					if (page) {
						auto block = blockLoad<adf::sBitmapBlock>(page);
						if (!block) {
							gDebug->error("bitmap block load error");
							return false;
						}
						mBitmapBlocks.push_back(block);
					}

				}
				blockBitmapExt = blockExt->nextBlock;
			}

			return true;
		}

		/**
		 * Save the disk bitmap
		 */
		bool cADF::filesystemBitmapSave() {
			return false;
		}

		bool cADF::blockBootLoad() {
			mBootBlock = blockLoad<adf::sBootBlock>(0);
			if (!mBootBlock)
				return false;

			// Original AmigaDOS 1.2 FS
			//if (!(mBootBlock->dosType[3] & eFlags::FFS))
			//	mBlockSize = 488;

			return true;
		}

		/**
		 * Locate and load the root block
		 */
		bool cADF::blockRootLoad() {
			switch (diskType()) {
			case adf::eType::FLOPPY_HD:
				mBlockLast = (80 * 2 * 12) - 1;
				break;
			case adf::eType::FLOPPY_DD:
				mBlockLast = (80 * 2 * 11) - 1;
				break;
			case adf::eType::HARDDRIVE:
				mBlockLast = (sourceSize() / blockSize()) - 1;
				break;
			default:
				return false;
			}

			mBlockFirst = 0;
			mBlockRoot = (mBlockLast + 1 - mBlockFirst) / 2;
			mRootBlock = blockLoad<adf::sRootBlock>(mBlockRoot);
			return (mRootBlock != 0);
		}

		/**
		 * Swap the endian in blocks
		 */
		template <class tBlockType> void cADF::blockSwapEndian(std::shared_ptr<tBlockType> pBlock) {

			if (typeid(tBlockType) == typeid(adf::sBootBlock))
				adf::blockSwapEndian((uint8_t*)pBlock.get(), 0);

			if (typeid(tBlockType) == typeid(adf::sRootBlock))
				adf::blockSwapEndian((uint8_t*)pBlock.get(), 1);

			if (typeid(tBlockType) == typeid(adf::sEntryBlock))
				adf::blockSwapEndian((uint8_t*)pBlock.get(), 3);

			if (typeid(tBlockType) == typeid(adf::sFileHeaderBlock))
				adf::blockSwapEndian((uint8_t*)pBlock.get(), 3);

			if (typeid(tBlockType) == typeid(adf::sOFSDataBlock))
				adf::blockSwapEndian((uint8_t*)pBlock.get(), 2);

			if (typeid(tBlockType) == typeid(adf::sFileExtBlock))
				adf::blockSwapEndian((uint8_t*)pBlock.get(), 5);

			if (typeid(tBlockType) == typeid(adf::sBitmapBlock))
				adf::blockSwapEndian((uint8_t*)pBlock.get(), 5);

			if (typeid(tBlockType) == typeid(adf::sBitmapExtBlock))
				adf::blockSwapEndian((uint8_t*)pBlock.get(), 5);
		}

		/**
		 * Load a block, dont validate checksum
		 */
		template <class tBlockType> std::shared_ptr<tBlockType> cADF::blockLoadNoCheck(const size_t pBlock) {
			auto block = blockObjectGet<tBlockType>(pBlock);
			if (!block)
				return 0;

			blockSwapEndian(block);
			return block;
		}

		/**
		 * Read a block
		 */
		template <class tBlockType> std::shared_ptr<tBlockType> cADF::blockLoad(const size_t pBlock) {
			auto block = blockObjectGet<tBlockType>(pBlock);
			if (!block)
				return 0;

			//  Checksum is generated before endian swap
			auto checksum = blockChecksum((const uint8_t*)block.get(), blockSize());

			if (typeid(tBlockType) == typeid(adf::sBootBlock)) {
				adf::sBootBlock* blockptr = (adf::sBootBlock*)block.get();

				checksum = blockBootChecksum((const uint8_t*)block.get(), blockSize());
				blockSwapEndian(block);

				if (blockptr->data[0] != 0) {
					if (checksum != block->checkSum) {
						gDebug->error("Boot Block checksum fail");
						return 0;
					}
				}
				return block;

			} else if (typeid(tBlockType) == typeid(adf::sBitmapBlock)) {
				checksum = blockChecksum((const uint8_t*)block.get(), blockSize(), 0);
			}

			blockSwapEndian(block);

			if (checksum != block->checkSum) {
				gDebug->error("Block checksum fail");
				return 0;
			}
			return block;
		}

		/**
		 * Write a block
		 */
		template <class tBlockType> bool cADF::blockSave(const size_t pBlock, std::shared_ptr<tBlockType> pData) {

			blockSwapEndian(pData);
			auto val = blockChecksum((const uint8_t*)pData.get(), blockSize());

			pData->checkSum = readBEDWord(&val);

			return blockObjectPut< tBlockType>(pBlock, pData);
		}
	}
}
