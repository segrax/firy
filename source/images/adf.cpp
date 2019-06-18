#include "firy.hpp"
#include "adf.hpp"

namespace firy {
	namespace images {

		using namespace adf;

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


			/*
				* swapEndian
				*
				* magic :-) endian swap function (big -> little for read, little to big for write)
				*/

			void swapEndian(uint8_t* buf, int type) {
				int i, j;
				int p;

				i = 0;
				p = 0;

				while (swapTable[type][i] != 0) {
					for (j = 0; j < swapTable[type][i]; j++) {
						switch (swapTable[type][i + 1]) {
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

				if (p != swapTable[type][i + 1])
					printf("Warning: Endian Swapping length");		/* BV */
			}
		}

		/**
		 * Maximum number of bytes which can be stored in a block
		 */
		const size_t gBytesPerBlock = 512;

		cADF::cADF() : cDisk<interfaces::cBlocks>() {
			mBlockSize = gBytesPerBlock;

			mBlockFirst = 0;
			mBlockLast = 0;
			mBlockRoot = 0;
		}

		uint32_t cADF::blockChecksum(const uint8_t* pBuffer, const size_t pBufferLen) {
			int32_t newsum = 0;
			for (size_t i = 0; i < (pBufferLen / 4); i++)
				if (i != (20 / 4))
					newsum += readBEDWord(pBuffer + i * 4);

			return -newsum;
		}

		bool cADF::filesystemPrepare() {

			if (!blockBootLoad())
				return false;
			if (!blockRootLoad())
				return false;



			return true;
		}

		spBuffer cADF::filesystemRead(spNode pFile) {
			return {};
		}

		spBuffer cADF::blockRead(const tBlock pBlock) {
			return {};
		}

		bool cADF::blockWrite(const tBlock pBlock, const spBuffer pBuffer) {
			return false;
		}

		size_t cADF::blockSize(const tBlock pBlock) const {
			return gBytesPerBlock;
		}

		adf::eType cADF::diskType() const {

			if ((mBuffer->size() >= 512 * 11 * 2 * 80) ||
				(mBuffer->size() <= 512 * 11 * 2 * 83))
				return(eType::FLOPPY_DD);

			else if (mBuffer->size() == 512 * 22 * 2 * 80)
				return(eType::FLOPPY_HD);

			else if (mBuffer->size() > 512 * 22 * 2 * 80)
				return(eType::HARDDRIVE);
			
			return eType::UNKNOWN;
		}

		bool cADF::filesystemChainLoad(spFile pFile) {
			return false;
		}
		bool cADF::blockBootLoad() {
			memcpy((void*)& mBootBlock, getBufferPtr(0), sizeof(adf::sBootBlock));
			swapEndian((uint8_t*)& mBootBlock, 0);

			if (!(mBootBlock.dosType[3] & eFlags::FFS))
				mBlockSize = 488;

			return true;
		}

		bool cADF::blockRootLoad() {

			switch (diskType()) {
			case adf::eType::FLOPPY_HD:
				return blockRootFloppyLoad(12);

			case adf::eType::FLOPPY_DD:
				return blockRootFloppyLoad(11);

			case adf::eType::HARDDRIVE:
				return blockRootHarddriveLoad();

			default:
				return false;
			}
		}

		bool cADF::blockRootFloppyLoad(const tBlock pSectors) {
			mBlockFirst = 0;
			mBlockLast = (80 * 2 * pSectors) - 1;
			mBlockRoot = (mBlockLast + 1 - mBlockFirst) / 2;

			auto blockPtr = getBufferPtr(mBlockRoot * blockSize());

			memcpy((void*)& mRootBlock, blockPtr, sizeof(adf::sRootBlock));
			swapEndian((uint8_t*)& mRootBlock, 1);

			auto checksum = blockChecksum(blockPtr, gBytesPerBlock);
			if (checksum != mRootBlock.checkSum) {
				std::cout << "Block checksum fail\n";
				return false;
			}

			return true;
		}

		bool cADF::blockRootHarddriveLoad() {
			return false;
		}
	}
}
