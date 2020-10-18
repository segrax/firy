#include <fstream>

namespace firy {

	class cConsole {

	public:
		cConsole(const int pLevel);

		std::string output() {
			return "\n";
		}

		void levelSet(const int pLevel) { mLevel = pLevel; }
		int levelGet() { return mLevel; }


		template<typename tMessage, typename ... tStrings>
		void error(const tMessage pModule, const tMessage pMessage, const tStrings ...pRest) {
			outputTime("error", pModule, pMessage, pRest...);;;
		}

		template<typename tMessage, typename ... tStrings>
		void notice(const tMessage pModule, const tMessage pMessage, const tStrings ...pRest) {
			if (mLevel >= 1)
				outputTime("notice", pModule, pMessage, pRest...);

			return { true };
		}

		/**
		 * Can abort
		 */
		template<typename tMessage, typename ... tStrings>
		void warning(const tMessage pModule, const tMessage pMessage, const tStrings ...pRest) {
			if (mLevel > 1)
				outputTime("warning", pModule, pMessage, pRest...);
		}

	private:

		template<typename tMessage, typename ... tStrings>
		std::string output(const tMessage pMessage, const tStrings& ...pRest) {
			return pMessage + output(pRest...);
		}

		template<typename tMessage, typename ... tStrings>
		void outputTime(const std::string pType, const tMessage pModule, const tMessage pMessage, const tStrings& ...pRest) {

			std::cerr << +": [" + pType + "] [" + pModule + "] " + output(pMessage, pRest...) << std::endl;
		}

		std::string getTime() const;

		int mLevel;
	};

	extern std::shared_ptr<firy::cConsole> gConsole;
}

