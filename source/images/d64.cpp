#include "firy.hpp"
#include "d64.hpp"

namespace firy {
	namespace images {

		/**
		 * Maximum number of bytes which can be stored in a block
		 */
		const size_t gBytesPerSector = 254;

		/**
		 * 
		 */
		std::string stringRip(const uint8_t* pBuffer, uint8_t pTerminator, size_t pLengthMax) {
			std::string tmpString;

			for (size_t i = 0; *pBuffer != pTerminator && i <= pLengthMax; ++i)
				tmpString += (char)* pBuffer++;

			return tmpString;
		}

		/**
		 * D64File Constructor
		 */
		sD64File::sD64File(wpFilesystem pFilesystem) : sFile(pFilesystem) {
			mType = eD64FileType_DEL;
			mSizeInSectors = 0;
		}

		/**
		 * D64File Read
		 */
		spBuffer sD64File::read() {

			return mFilesystem.lock()->filesystemRead(shared_from_this());
		}

		/**
		 * D64 Constructor
		 */
		cD64::cD64() : cDisk<interfaces::cTracks>() {

			mTrackCount = 35;
		}

		/**
		 * Load the D64 directory
		 */
		bool cD64::filesystemPrepare() {
			tTrackSector ts(18, 1);

			mFsRoot = std::make_shared<firy::filesystem::sDirectory>( weak_from_this() );
			mFsRoot->mName = "/";

			// Loop until we reach the end of the directory
			while (	(ts.first > 0 && ts.first <= trackCount()) && 
					(ts.second <= sectorCount(ts.first))) {

				auto sectorBuffer = getBufferPtr(sectorOffset(ts));
				auto buffer = sectorBuffer;
				if (!buffer)
					break;

				// 8 entries per sector, 0x20 bytes per entry
				for (uint8_t i = 0; i <= 7; ++i, buffer += 0x20) {
					spD64File file = filesystemEntryProcess(buffer);
					if (file)
						mFsRoot->mNodes.push_back(file);
				}

				// Get the next Track/Sector in the chain
				tTrackSector nextTs = { sectorBuffer[0], sectorBuffer[1] };
				if (nextTs == ts)
					break;
				ts = std::move(nextTs);
			}
			return true;
		}

		/**
		 * Load a file off the D64
		 */
		spBuffer cD64::filesystemRead(spNode pNode) {
			uint16_t bytesCopied = 0, copySize = gBytesPerSector;
			spD64File File = std::dynamic_pointer_cast<sD64File>(pNode);
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

				uint8_t* sectorptr = getBufferPtr(sectorOffset(ts));
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
					copySize = (uint16_t) (ts.second - 1);
					// Adjust bufer size to match the final file size
					buffer->resize(buffer->size() - (gBytesPerSector - copySize));
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
		 * Read an entire track
		 */
		spBuffer cD64::trackRead(const tTrack pTrack) {
			auto sectorBuffer = getBufferPtr(trackOffset(pTrack));
			if (!sectorBuffer)
				return {};

			auto buffer = std::make_shared<tBuffer>();
			buffer->insert(buffer->begin(), sectorBuffer, sectorBuffer + trackSize(pTrack));
			return buffer;
		}

		bool cD64::trackWrite(const tTrack pBlock, const spBuffer pBuffer) {

			return false;
		}

		spBuffer cD64::sectorRead(const tTrackSector pTS) {
			auto sectorBuffer = getBufferPtr(sectorOffset(pTS));
			if (!sectorBuffer)
				return {};

			auto buffer = std::make_shared<tBuffer>();
			buffer->insert(buffer->begin(), sectorBuffer, sectorBuffer + sectorSize(pTS.first));
			return buffer;
		}

		bool cD64::sectorWrite(const tTrackSector pTS, const spBuffer pBuffer) {

			return false;
		}

		tSector cD64::sectorCount(const tTrack pTrack) const {
			return (21 - (pTrack > 17) * 2 - (pTrack > 24) - (pTrack > 30));
		}
		
		size_t cD64::sectorSize(const tTrack pTrack) const {
			return 256;
		}

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

				uint8_t* sectorptr = getBufferPtr(sectorOffset(ts));
				if (!sectorptr) {
					pFile->mChainBroken = true;
					return false;
				}

				// Next Track/Sector for this file
				ts = { sectorptr[0], sectorptr[1] };
			}
			return true;
		}

		spD64File cD64::filesystemEntryProcess(const uint8_t* pBuffer) {
			spD64File file = std::make_shared<sD64File>( weak_from_this() );

			// Get the filetype
			file->mType = (eD64FileType)(pBuffer[0x02] & 0x0F);

			// Get the filename
			file->mName = stringRip(pBuffer + 0x05, 0xA0, 16);
			if (file->mName.size() == 0) {
				return 0;
			}

			// Get the starting Track/Sector
			file->mChain.emplace_back(pBuffer[0x03], pBuffer[0x04]);

			// Total number of blocks
			file->mSizeInSectors = readLEWord(&pBuffer[0x1E]);
			file->mSizeInBytes = readLEWord(&pBuffer[0x1E]) * (sectorSize() - 2);

			return file;
		}
	}
}