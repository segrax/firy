
namespace firy {
	class cOptions;

	/**
	 * Response to a failure
	 */
	class cOptionResponse {
	public:
		inline bool isAborted() const { return mAbort; }
		cOptionResponse(bool pAbort) { mAbort = pAbort; }

	protected:
		bool mAbort;
	};

	using spOptionResponse = std::shared_ptr<cOptionResponse>;
	using spOptions = std::shared_ptr<cOptions>;
	
	/**
	 * User options
	 */
	class cOptions : public std::enable_shared_from_this<cOptions> {

	public:
		cOptions();

		virtual spOptionResponse warning(pImage pImage, const std::string& pMessage, const std::string& pMessageDetail = "");
		virtual void error(pImage pImage, const std::string& pMessage, const std::string& pMessageDetail = "");
		virtual spOptionResponse savechanges(pImage pImage, const std::string& pMessage);
		virtual spOptionResponse savechangesExit(pImage pImage, const std::string& pMessage);


		virtual void errorShowSet(const bool pEnabled);
		virtual void warningShowSet(const bool pEnabled);


		virtual spOptions clone() { return std::make_shared<cOptions>(*shared_from_this()); };

	protected:
		bool mAutoSaveSource;
		bool mAutoSaveSourceExit;
		bool mWarningShow;
		bool mErrorShow;

	private:
		
	};


}
