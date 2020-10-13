namespace firy {

	namespace sources {

		/**
		 * Access a file
		 */
		class cFile : public cInterface {
		public:

			cFile(size_t pChunkSize = gMegabyte);

			virtual bool create(const std::string pFile) override;
			virtual bool open(const std::string pFile) override;
			virtual void close() override;
			virtual bool save(const std::string pFile) override;

			virtual spBuffer chunk(const size_t pFileOffset = 0) override;


		private:
			std::string mFilename;					// Path/Name of image file
		};

		/**
		 * File shared pointer
		 */
		typedef std::shared_ptr<cInterface> spSourceFile;

	}
}