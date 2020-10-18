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
			~cImage();

			bool filesystemSave();

			/**
			 * Name of image type
			 */
			virtual std::string imageType() const = 0;

			/**
			 * Short name of image
			 */
			virtual std::string imageTypeShort() const = 0;

			/**
			 * Warning wrapper
			 */
			virtual spOptionResponse warning(const std::string& pMessage) {
				return mOptions->warning(this, pMessage);
			}

			/**
			 * Error wrapper
			 */
			virtual void error(const std::string& pMessage, const std::string& pMessageDetail = "") {
				mOptions->error(this, pMessage, pMessageDetail);
			}

			virtual spOptionResponse savechanges(const std::string& pMessage) {
				return mOptions->savechanges(this, pMessage);
			}

			virtual spOptionResponse savechangesExit(const std::string& pMessage) {
				return mOptions->savechangesExit(this, pMessage);
			}

			/**
			 * Set options
			 */
			virtual void optionsSet(spOptions pOptions);

			/** 
		     * Create a file in the filesystem native class, returning a generic pointer
			 */
			virtual spFile filesystemFileCreate(const std::string& pName = "") = 0;

			/**
			 * Create a directory in the filesystem native class, returning a generic pointer
			 */
			virtual spDirectory filesystemDirectoryCreate(const std::string& pName = "") = 0;

			/**
			 * Create a file attached to this filesystem
			 */
			template <class tType, class ...Args> std::shared_ptr<tType> filesystemNodeCreate(const std::string& pName = "", ...) {
				auto res = std::make_shared<tType>(weak_from_this(), pName, Args...);
				res->dirty(true);
				return res;
			}

		private:
			spOptions mOptions;

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

	using spImage = std::shared_ptr<images::cImage>;
	using wpImage = std::weak_ptr<images::cImage>;
}
