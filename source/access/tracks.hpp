namespace firy {

	typedef size_t tTrack;
	typedef size_t tSector;

	typedef std::pair<tTrack, tSector> tTrackSector;

	namespace access {

		/**
		 * Provide a track/sector read/write interface
		 */
		class cTracks : public virtual access::cInterface {
		public:

			/**
			 * Constructor
			 */
			cTracks();

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
			 * Is a T/S free
			 */
			virtual bool sectorIsFree(const tTrackSector pTS) const = 0;

			/**
			 * Set a T/S used
			 */
			virtual bool sectorSet(const tTrackSector pTS, const bool pValue) = 0;

			/**
			 * Get 'pTotal' number of free sectors, marking them used
			 */
			virtual std::vector<tTrackSector> sectorsUse(const tSector pTotal) = 0;

			/**
			 * Free all sectors in 'pSectors'
			 */
			virtual bool sectorsFree(const std::vector<tTrackSector>& pSectors) = 0;

			/**
			 * Get the free sectors on a track
			 */
			virtual std::vector<tTrackSector> sectorsGetFree(const tTrack pTrack = 0) const = 0;

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
