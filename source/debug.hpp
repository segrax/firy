#include <fstream>

namespace firy {
	class cDebug {

	public:
		cDebug(const int pLevel);

		std::string output() {
			return "\n";
		}

		void outputDisable(const bool pVal) { mDisable = pVal; }
		void levelSet(const int pLevel) { mLevel = pLevel; }
		int levelGet() { return mLevel; }

		template<typename tMessage, typename ... tStrings>
		std::string output(const tMessage pMessage, const tStrings& ...pRest) {
			return pMessage + output(pRest...);
		}

		template<typename tMessage, typename ... tStrings>
		void outputTime(const std::string pType, const tMessage pMessage, const tStrings& ...pRest) {
			if (mDisable)
				return;

			std::cerr << getTime() + ": [" + pType + "] " + output(pMessage, pRest...);

			//std::ofstream outfile;
			//outfile.open("log.txt", std::ios_base::app); // append instead of overwrite
			//outfile << getTime() + ": [" + pType + "] " + output(pMessage, pRest...);
		}

		template<typename tMessage, typename ... tStrings>
		void error(const tMessage pMessage, const tStrings ...pRest) {
			if (mLevel >= 0)
				outputTime("error", pMessage, pRest...);
		}

		template<typename tMessage, typename ... tStrings>
		void notice(const tMessage pMessage, const tStrings ...pRest) {
			if (mLevel > 1)
				outputTime("notice", pMessage, pRest...);
		}

	private:

		std::string getTime() const;

		int mLevel;
		bool mDisable;
	};

	extern std::shared_ptr<firy::cDebug> gDebug;
}

