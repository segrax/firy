namespace firy {
	namespace images {

		/**
		 * Image that contains a filesystem and a source interface
		 */
		class cImage : public virtual access::cInterface,
					   public filesystem::cInterface,
					   public std::enable_shared_from_this<cImage> {

		public:

			cImage();

			/**
			 * Name of image type
			 */
			virtual std::string imageType() const = 0;

			/**
			 * Common file extensions
			 */
			virtual std::vector<std::string> imageExtensions() const = 0;
		};

		/**
		 * Image with an access interface
		 */
		template <class tAccessInterface> class cImageAccess : 
			public cImage,
			public tAccessInterface {

		public:

			cImageAccess() : 
				cImage(), 
				tAccessInterface() {
			};

		};
	}

	typedef std::shared_ptr<images::cImage> spImage;
	template <class tImageType>
		using spImageType = std::shared_ptr<tImageType>;
}
