#include "firy.hpp"
#include "adf.hpp"

namespace firy {
	namespace images {

		namespace adf {

#define SW_LONG  4
#define SW_SHORT 2
#define SW_CHAR  1

			int swapTable[][15] = {
				{ 4, SW_CHAR, 2, SW_LONG, 1012, SW_CHAR, 0, 1024 },     /* first bytes of boot */
				{ 108, SW_LONG, 40, SW_CHAR, 10, SW_LONG, 0, 512 },        /* root */
				{ 6, SW_LONG, 488, SW_CHAR, 0, 512 },                      /* data */
																		/* file, dir, entry */
				{ 82, SW_LONG, 92, SW_CHAR, 3, SW_LONG, 36, SW_CHAR, 11, SW_LONG, 0, 512 },
				{ 6, SW_LONG, 0, 24 },                                       /* cache */
				{ 128, SW_LONG, 0, 512 },                                /* bitmap, fext */
																		/* link */
				{ 6, SW_LONG, 64, SW_CHAR, 86, SW_LONG, 32, SW_CHAR, 12, SW_LONG, 0, 512 },
				{ 4, SW_CHAR, 39, SW_LONG, 56, SW_CHAR, 10, SW_LONG, 0, 256 }, /* RDSK */
				{ 4, SW_CHAR, 127, SW_LONG, 0, 512 },                          /* BADB */
				{ 4, SW_CHAR, 8, SW_LONG, 32, SW_CHAR, 31, SW_LONG, 4, SW_CHAR, /* PART */
				  15, SW_LONG, 0, 256 },
				{ 4, SW_CHAR, 7, SW_LONG, 4, SW_CHAR, 55, SW_LONG, 0, 256 }, /* FSHD */
				{ 4, SW_CHAR, 4, SW_LONG, 492, SW_CHAR, 0, 512 }             /* LSEG */
			};

			bool adfIsLeap(int y) {
				return((bool)(!(y % 100) ? !(y % 400) : !(y % 4)));
			}

			void adfDays2Date(int32_t days, int* yy, int* mm, int* dd) {
				int y, m;
				int nd;
				int jm[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

				/* 0 = 1 Jan 1978,  6988 = 18 feb 1997 */

				/*--- year ---*/
				y = 1978;
				if (adfIsLeap(y))
					nd = 366;
				else
					nd = 365;
				while (days >= nd) {
					days -= nd;
					y++;
					if (adfIsLeap(y))
						nd = 366;
					else
						nd = 365;
				}

				/*--- month ---*/
				m = 1;
				if (adfIsLeap(y))
					jm[2 - 1] = 29;
				while (days >= jm[m - 1]) {
					days -= jm[m - 1];
					m++;
				}

				*yy = y;
				*mm = m;
				*dd = days + 1;
			}

			/*
				* swapEndian
				*
				* magic :-) endian swap function (big -> little for read, little to big for write)
				*/
			void swapEndian(uint8_t* buf, int mType) {
				int i = 0, j = 0;
				int p = 0;

				while (swapTable[mType][i] != 0) {
					for (j = 0; j < swapTable[mType][i]; j++) {
						switch (swapTable[mType][i + 1]) {
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

				if (p != swapTable[mType][i + 1])
					printf("Warning: Endian Swapping length");		/* BV */
			}


			/**
			 * Maximum number of bytes which can be stored in a block
			 */
			const size_t gBytesPerBlock = 512;

			sADFFile::sADFFile(wpFilesystem pFilesystem) : sEntry(), sFile(pFilesystem) {

			}

			sADFDir::sADFDir(wpFilesystem pFilesystem) : sEntry(), sDirectory(pFilesystem) {

			}
		}

		cADF::cADF(spSource pSource) : cImageAccess<access::cBlocks>(), access::cInterface(pSource) {
			mBlockSize = adf::gBytesPerBlock;

			mBlockFirst = 0;
			mBlockLast = 0;
			mBlockRoot = 0;
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
		bool cADF::filesystemPrepare() {
			if (!blockBootLoad())
				return false;
			if (!blockRootLoad())
				return false;

			if (!filesystemBitmapLoad())
				return false;

			auto Root = std::make_shared<adf::sADFDir>(weak_from_this());
			Root->mBlock = mBlockRoot;

			mFsRoot = Root;
			return entrysLoad(Root);
		}

		/**
		 * Load the contents of a directory
		 */
		bool cADF::entrysLoad(adf::spADFDir pDir) {
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
			adf::sEntry* entry = 0;

			switch (blockEntry->secType) {
			case 1:		// ST_ROOT
				return 0;
			case 2: 	// ST_DIR
			case 4:	{	// ST_LDIR
				auto Dir = std::make_shared<adf::sADFDir>(weak_from_this());
				node = Dir;
				entry = Dir.operator->();
				entry->mReal = blockEntry->realEntry;
				break;
			}

			case 3:		// ST_LSOFT
			case -3: 	// ST_FILE
			case -4: {	// ST_LFILE
				auto File = std::make_shared<adf::sADFFile>(weak_from_this());
				node = File;
				entry = File.operator->();
				entry->mReal = blockEntry->realEntry;
				break;
			}
			
			default:
				return 0;	// TODO
				break;
			}

			entry->mType = blockEntry->secType;
			entry->mParent = blockEntry->parent;

			node->mName = std::string(blockEntry->name, min(blockEntry->nameLen, adf::MAXNAMELEN));

			adf::adfDays2Date(blockEntry->days, &(entry->year), &(entry->month), &(entry->days));
			entry->hour = blockEntry->mins / 60;
			entry->mins = blockEntry->mins % 60;
			entry->secs = blockEntry->ticks / 50;

			entry->mBlock = pBlock;
			entry->mNextSameHash = blockEntry->nextSameHash;

			if(blockEntry->secType == 2)
				entrysLoad(std::dynamic_pointer_cast<adf::sADFDir>(node));

			return node;
		}

		spBuffer cADF::filesystemRead(spNode pFile) {
			adf::spADFFile File = std::dynamic_pointer_cast<adf::sADFFile>(pFile);
			if (!File)
				return {};

			auto blockFile = blockLoad<adf::sFileHeaderBlock>(File->mBlock);
			size_t totalbytes = blockFile->byteSize;

			spBuffer buffer = std::make_shared<tBuffer>();
			buffer->resize(blockFile->byteSize);
			uint8_t *destptr = buffer->data();

			// Old 1.2 Filesystem
			if (!(mBootBlock->dosType[3] & adf::eFlags::FFS)) {
				tBlock currentblock = blockFile->firstData;
				int count = 1;

				while (currentblock) {
					auto block = blockLoad<adf::sOFSDataBlock>(currentblock);

					memcpy(destptr, block->data, block->dataSize);
					destptr += block->dataSize;
					totalbytes -= block->dataSize;
					if (!totalbytes)
						return buffer;

					if (block->seqNum + 1 != ++count) {
						// TODO: Error (Sequence is wrong)
						return {};
					}
					currentblock = block->nextData;
				}

			} else {
				for (int index = adf::MAX_DATABLK-1; index >= 0; --index) {
					if (!blockFile->dataBlocks[index])
						break;

					auto block = chunkPtr(blockFile->dataBlocks[index] * blockSize());
					auto size = min(totalbytes, blockSize());
					memcpy(destptr, block, size);
					destptr += size;
					totalbytes -= size;
				}

				tBlock nextSector = blockFile->extension;
				while (nextSector) {
					auto blockExt = blockLoad<adf::sFileExtBlock>(nextSector);
					if (!blockExt) {
						// TODO: Error
						break;
					}

					for (int index = adf::MAX_DATABLK - 1; index >= 0; --index) {
						if (!blockExt->dataBlocks[index])
							break;

						auto block = chunkPtr(blockExt->dataBlocks[index] * blockSize());
						auto size = min(totalbytes, blockSize());
						memcpy(destptr, block, size);
						destptr += size;
						totalbytes -= size;
					}

					nextSector = blockExt->extension;
				}
			}

			return buffer;
		}

		size_t cADF::blockSize(const tBlock pBlock) const {
			return adf::gBytesPerBlock;
		}

		bool cADF::blockIsFree(const tBlock pBlock) const {
			tBlock sectOfMap = pBlock - 2;
			tBlock block = sectOfMap / (127 * 32);
			tBlock indexInMap = (sectOfMap / 32) % 127;
			static uint32_t bitMask[32] = {
				0x1, 0x2, 0x4, 0x8,
				0x10, 0x20, 0x40, 0x80,
				0x100, 0x200, 0x400, 0x800,
				0x1000, 0x2000, 0x4000, 0x8000,
				0x10000, 0x20000, 0x40000, 0x80000,
				0x100000, 0x200000, 0x400000, 0x800000,
				0x1000000, 0x2000000, 0x4000000, 0x8000000,
				0x10000000, 0x20000000, 0x40000000, 0x80000000 };

			return ((mBitmapBlocks[block]->map[indexInMap]
				& bitMask[sectOfMap % 32]) != 0);
		}

		std::vector<tBlock> cADF::blocksFree() const {
			std::vector<tBlock> freeBlocks;
			for (tBlock j = mBlockFirst + 2; j <= (mBlockLast - mBlockFirst); j++)
				if (blockIsFree(j))
					freeBlocks.push_back(j);

			return freeBlocks;
		}

		std::shared_ptr<adf::sOFSDataBlock> cADF::blockReadOFS(const tBlock pBlock) {
			return blockLoad<adf::sOFSDataBlock>(pBlock);
		}

		adf::eType cADF::diskType() const {
			if ((sourceSize() >= 512 * 11 * 2 * 80) &&
				(sourceSize() <= 512 * 11 * 2 * 83))
				return(adf::eType::FLOPPY_DD);

			else if (sourceSize() == 512 * 22 * 2 * 80)
				return(adf::eType::FLOPPY_HD);

			else if (sourceSize() > 512 * 22 * 2 * 80)
				return(adf::eType::HARDDRIVE);
			
			return adf::eType::UNKNOWN;
		}

		std::string cADF::filesystemNameGet() const {
			return std::string(mRootBlock->diskName, mRootBlock->nameLen);
		}

		bool cADF::filesystemChainLoad(spFile pFile) {
			return false;
		}
		
		bool cADF::filesystemBitmapLoad() {

			mBitmapBlocks.clear();

			for (auto& page : mRootBlock->bmPages) {
				if (page) {
					auto block = blockLoad<adf::sBitmapBlock>(page);
					if (!block) {
						std::cout << "bitmap block load error\n";
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
							std::cout << "bitmap block load error\n";
							return false;
						}
						mBitmapBlocks.push_back(block);
					}

				}
				blockBitmapExt = blockExt->nextBlock;
			}

			return true;
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
				mBlockLast = (sourceSize() / 512) - 1;
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
				adf::swapEndian((uint8_t*)pBlock.get(), 0);

			if (typeid(tBlockType) == typeid(adf::sRootBlock))
				adf::swapEndian((uint8_t*)pBlock.get(), 1);

			if (typeid(tBlockType) == typeid(adf::sEntryBlock))
				adf::swapEndian((uint8_t*)pBlock.get(), 3);

			if (typeid(tBlockType) == typeid(adf::sFileHeaderBlock))
				adf::swapEndian((uint8_t*)pBlock.get(), 3);

			if (typeid(tBlockType) == typeid(adf::sOFSDataBlock))
				adf::swapEndian((uint8_t*)pBlock.get(), 2);

			if (typeid(tBlockType) == typeid(adf::sFileExtBlock))
				adf::swapEndian((uint8_t*)pBlock.get(), 5);

			if (typeid(tBlockType) == typeid(adf::sBitmapBlock))
				adf::swapEndian((uint8_t*)pBlock.get(), 5);

			if (typeid(tBlockType) == typeid(adf::sBitmapExtBlock))
				adf::swapEndian((uint8_t*)pBlock.get(), 5);
		}

		/**
		 * 
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
						std::cout << "Block checksum fail\n";
						//return 0;
					}
				}
				return block;

			} else if (typeid(tBlockType) == typeid(adf::sBitmapBlock)) {
				checksum = blockChecksum((const uint8_t*)block.get(), blockSize(), 0);
			}

			blockSwapEndian(block);

			if (checksum != block->checkSum) {
				std::cout << "Block checksum fail\n";
				return 0;
			}
			return block;
		}
	}
}
