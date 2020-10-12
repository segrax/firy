namespace firy {

	namespace sources {

		/**
		 * Access a file
		 */
		class cFile : public cInterface {
		public:

			cFile(size_t pChunkSize = gMegabyte);

			virtual bool open(const std::string pFile);
			virtual void close();
			virtual bool save(const std::string pFile);

			virtual spBuffer chunk(const size_t pFileOffset = 0);


		private:
			std::string mFilename;					// Path/Name of image file
		};

		/**
		 * File shared pointer
		 */
		typedef std::shared_ptr<cInterface> spSourceFile;

	}
}