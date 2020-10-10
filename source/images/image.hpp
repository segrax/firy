namespace firy {
	namespace images {

		class cImage : public virtual access::cInterface,
					   public filesystem::cInterface,
					   public std::enable_shared_from_this<cImage> {

		public:
			cImage() :
				access::cInterface(0),
				filesystem::cInterface(),
				std::enable_shared_from_this<cImage>() {

			};


			virtual std::string filesystemNameGet() const {
				return "";
			}
		};

		template <class tAccessInterface> class cImageAccess : 
			public cImage,
			public tAccessInterface {

		public:

			cImageAccess() : 
				cImage(), 
				tAccessInterface() {
			};

		};

		using spImage = std::shared_ptr<cImage>;
	}
}
