/*
 *  FIRY
 *  ---------------
 *
 *  Copyright (C) 2019-2021
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

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
