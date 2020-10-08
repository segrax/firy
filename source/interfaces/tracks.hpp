namespace firy {

	typedef size_t tTrack;
	typedef size_t tSector;

	typedef std::pair<tTrack, tSector> tTrackSector;

	namespace interfaces {

		/**
		 * Provide a track/sector read/write interface
		 */
		class cTracks : public interfaces::cSource {
		public:

			/**
			 * Constructor
			 */
			cTracks(spSource pSource);

			/**
			 * Read the provided track
			 */
			virtual spBuffer trackRead(const tTrack pTrack);

			/**
			 * Write 'pBuffer' at provided track
			 */
			virtual bool trackWrite(const tTrack pTrack, const spBuffer pBuffer);

			/**
			 * Number of tracks
			 */
			virtual tTrack trackCount() const;

			/**
			 * Number of bytes per track
			 */
			virtual size_t trackSize(const tTrack pTrack = 0) const;
			
			/**
			 * Read the provided T/S
			 */
			virtual spBuffer sectorRead(const tTrackSector pTS);

			/**
			 * Write 'pBuffer' at provided T/S
			 */
			virtual bool sectorWrite(const tTrackSector pTS, const spBuffer pBuffer);

			/**
			 * Number of sectors per track
			 */
			virtual tSector sectorCount(const tTrack pTrack = 0) const = 0;

			/**
			 * Number of bytes per sector
			 */
			virtual size_t sectorSize(const tTrack pTrack = 0) const = 0;
			virtual size_t blockSize(const tTrack pTrack = 0) const { return sectorSize(pTrack); };

		protected:

			/**
			 * Get the offset from the start of the image, to the track
			 */
			virtual size_t trackOffset(const tTrack pTrack) const;
			
			/**
			 * Get the offset from the start of the image, to the track/sector
			 */
			virtual size_t sectorOffset(const tTrackSector pTS) const;

			/**
			 * Number of tracks
			 */
			tTrack mTrackCount;
		};
	}
}
