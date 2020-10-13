namespace firy {
	namespace images {

		/**
		 * Image that contains a filesystem and a source interface
		 */
		class cImage : public virtual access::cInterface,
					   public filesystem::cInterface,
					   public std::enable_shared_from_this<cImage>,
					   public virtual firy::helpers::cDirty {

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

			/**
			 * Create a file attached to this filesystem
			 */
			template <class tType, class ...Args> std::shared_ptr<tType> filesystemFileCreate(const std::string& pName = "", ...) {
				auto res = std::make_shared<tType>(weak_from_this(), pName, Args...);
				res->dirty(true);
				return res;
			}

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
}
