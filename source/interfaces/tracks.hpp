namespace firy {

	typedef size_t tTrack;
	typedef size_t tSector;

	typedef std::pair<tTrack, tSector> tTrackSector;

	namespace interfaces {

		/**
		 * Provide a track/sector read/write interface
		 */
		class cTracks {
		public:

			/**
			 * Constructor
			 */
			cTracks();

			/**
			 * Read the provided T/S
			 */
			virtual spBuffer trackRead(const tTrackSector pTS) = 0;

			/**
			 * Write 'pBuffer' at provided T/S
			 */
			virtual bool trackWrite(const tTrackSector pTS, const spBuffer pBuffer) = 0;

			/**
			 * Number of tracks
			 */
			virtual tTrack trackCount() const;

			/**
			 * Number of bytes per track
			 */
			virtual size_t trackSize(const tTrack pTrack = 0) const;

			/**
			 * Number of sectors per track
			 */
			virtual tSector sectorCount(const tTrack pTrack = 0) const = 0;

			/**
			 * Number of bytes per sector
			 */
			virtual size_t sectorSize(const tTrack pTrack = 0) const = 0;

		protected:

			/**
			 * Get the offset from the start of the image, to the track
			 */
			virtual size_t trackOffset(const tTrackSector pTrack) const;
			
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