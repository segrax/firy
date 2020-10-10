#include "firy.hpp"
#include "d64.hpp"

namespace firy {
	namespace images {
		namespace d64 {
			/**
			 * Maximum number of bytes which can be stored in a block
			 */
			const size_t gBytesPerSector = 254;

			/**
			 * D64File Constructor
			 */
			sFile::sFile(wpFilesystem pFilesystem) : filesystem::sFile(pFilesystem) {
				mType = eFileType_DEL;
				mSizeInSectors = 0;
			}
		}

		/**
		 * D64 Constructor
		 */
		cD64::cD64(spSource pSource) : cImageAccess<access::cTracks>(), access::cInterface(pSource) {

			// TODO: Determine tracks based on filesize
			mTrackCount = 35;
		}

		/**
		 * Get the name of the disk
		 */
		std::string cD64::filesystemNameGet() {
			return mLabel;
		}

		/**
		 * Load the D64 directory
		 */
		bool cD64::filesystemPrepare() {
			mFsRoot = std::make_shared<firy::filesystem::sDirectory>(weak_from_this());
			mFsRoot->mName = "/";

			// Loop until we reach the end of the directory
			tTrackSector ts(18, 1);
			while ((ts.first > 0 && ts.first <= trackCount()) &&
				(ts.second <= sectorCount(ts.first))) {

				auto sectorBuffer = sectorRead(ts);
				if (!sectorBuffer)
					break;

				// 8 entries per sector, 0x20 bytes per entry
				for (uint8_t i = 0; i <= 7; ++i) {
					d64::spFile file = filesystemEntryProcess(sectorBuffer, i * 0x20);
					if (file)
						mFsRoot->mNodes.push_back(file);
				}

				// Get the next Track/Sector in the chain
				tTrackSector nextTs = { sectorBuffer->getByte(0), sectorBuffer->getByte(1)};
				if (nextTs == ts)
					break;
				ts = std::move(nextTs);
			}
			return filesystemBitmapLoad();
		}

		/**
			* Load a file off the D64
			*/
		spBuffer cD64::filesystemRead(spNode pNode) {
			uint16_t bytesCopied = 0, copySize = d64::gBytesPerSector;
			d64::spFile File = std::dynamic_pointer_cast<d64::sFile>(pNode);
			if (!File)
				return 0;

			if (!filesystemChainLoad(File))
				return 0;

			// Prepare a buffer to hold the file
			spBuffer buffer = std::make_shared<tBuffer>();
			buffer->resize(File->mSizeInBytes);
			uint8_t* destBuffer = buffer->data();

			for (auto& ts : File->mChain) {
				bool noCopy = false;

				uint8_t* sectorptr = chunkPtr(sectorOffset(ts));
				if (!sectorptr) {
					File->mChainBroken = true;
					return buffer;
				}

				// T/S is broken, or the directory entry is wrong about size
				if (bytesCopied >= buffer->size())
					noCopy = true;

				// Last Sector of file? 
				if (!ts.first) {
					// Bytes used by the final sector stored in the T/S chain sector value
					copySize = (uint16_t)(ts.second - 1);
					// Adjust bufer size to match the final file size
					buffer->resize(buffer->size() - (d64::gBytesPerSector - copySize));
				}

				// Copy sector data, excluding the T/S Chain data
				if (!noCopy)
					memcpy(destBuffer, sectorptr + 2, copySize);

				// Move the dest buffer forward
				destBuffer += copySize;
				bytesCopied += copySize;
			}

			return buffer;
		}

		/**
		 * Calculate number of sectors for this track
		 */
		tSector cD64::sectorCount(const tTrack pTrack) const {
			return ((21 - (pTrack > 17) * 2) - (pTrack > 24) - (pTrack > 30));
		}

		/**
		 * Fixed sector size
		 */
		size_t cD64::sectorSize(const tTrack pTrack) const {
			return 256;
		}

		/**
		 * Load the T/S chain for a file
		 */
		bool cD64::filesystemChainLoad(spFile pFile) {
			tTrackSector ts = pFile->mChain[0];
			pFile->mChain.clear();

			while (ts.first) {
				pFile->mChain.push_back(ts);

				// Track/Sector already in use?
				//if (mBamRealTracks[currentTrack][currentSector]) {
					// Add to the crosslinked list
				//	mCrossLinked.push_back(sD64Chain(currentTrack, currentSector, pFile));
				//} else
				//	mBamRealTracks[currentTrack][currentSector] = pFile;

				uint8_t* sectorptr = chunkPtr(sectorOffset(ts));
				if (!sectorptr) {
					pFile->mChainBroken = true;
					return false;
				}

				// Next Track/Sector for this file
				ts = { sectorptr[0], sectorptr[1] };
			}
			return true;
		}

		/**
		 * Load the bitmap availability block
		 */
		bool cD64::filesystemBitmapLoad() {
			auto block = sectorRead({ 18,0 });

			mDosVersion = block->getByte(2);
			mDosType = block->getWordBE(0xA5);

			mLabel = block->getString(0x90, 16, 0xA0); 

			// Load the BAM (We are abusing the sector field to hold the byte offset
			tTrackSector ts = { 1, 4 };
			mBam.clear();
			for (; ts.first < 35; ++ts.first) {
				d64::sTrackBam bam;

				bam.mFreeSectors = block->getByte(ts.second++);
				bam.m0 = block->getByte(ts.second++);
				bam.m1 = block->getByte(ts.second++);
				bam.m2 = block->getByte(ts.second++);
				mBam.push_back(bam);
			}

			// Do we recognise the disk DOS type
			switch (mDosType) {
				default:	// Unknown DOS
					break;

				case '2A':	// CBM DOS v2.6
				case '2P':	// PrologicDOS, ProSpeed 
					return true;
			}

			return false;
		}

		/**
		 *
		 */
		d64::spFile cD64::filesystemEntryProcess(spBuffer pBuffer, size_t pOffset) {
			d64::spFile file = std::make_shared<d64::sFile>(weak_from_this());

			// Get the filetype
			file->mType = (d64::eFileType)(pBuffer->getByte(pOffset + 2) & 0x0F);

			// Get the filename
			file->mName = pBuffer->getString(pOffset + 0x05, 16, 0xA0);
			if (file->mName.size() == 0) {
				return 0;
			}

			// Get the starting Track/Sector
			file->mChain.emplace_back( pBuffer->getByte(pOffset + 3), pBuffer->getByte(pOffset + 4));

			// Total number of blocks
			file->mSizeInSectors = pBuffer->getWordLE(pOffset + 0x1E);
			file->mSizeInBytes = file->mSizeInSectors * (sectorSize() - 2);

			return file;
		}
	}
}
