#include <time.h>

namespace firy {
	namespace helpers {

		struct sDateTime {
			int year, month, days, hour, mins, secs;

			/**
			 * Default datetime to now
			 */
			sDateTime() {
				struct tm local;
				time_t cal;

				time(&cal);
				localtime_s(&local, &cal);

				year = local.tm_year + 1900;         /* since 1900 */
				month = local.tm_mon + 1;
				days = local.tm_mday;
				hour = local.tm_hour;
				mins = local.tm_min;
				secs = local.tm_sec;
			}
			
			/**
			 * Initialise at a specific date/time
			 */
			sDateTime(const int pYear, const int pMonth, const int pDay, const int pHour = 0, const int pMins = 0, const int pSecs = 0) {
				year = pYear;
				month = pMonth;
				days = pDay;
				hour = pHour;
				mins = pMins;
				secs = pSecs;
			}

			/**
			 * Reset to zero
			 */
			void reset() {
				year = month = days = hour = mins = secs = 0;
			}

		};
	}
}
