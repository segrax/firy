namespace firy {

	namespace sources {

		/**
		 * Access a file
		 */
		class cFile : public cSource {
		public:

			cFile(size_t pChunkSize = gMegabyte);

			virtual bool open(const std::string pFile);
			virtual void close();

			virtual spBuffer bufferCopy(const size_t pOffset = 0, const size_t pSize = 0);


			virtual spBuffer bufferChunk(const size_t pFileOffset = 0);


		private:
			std::string mSourceFilename;					// Path/Name of image file
		};

		/**
		 * File shared pointer
		 */
		typedef std::shared_ptr<cFile> spSourceFile;

		/**
		 * Open a file
		 */
		spSourceFile OpenFile(const std::string& pFile);
	}
}