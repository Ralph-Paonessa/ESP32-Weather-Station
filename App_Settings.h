// App_Settings.h

#ifndef _APP_SETTINGS_h
#define _APP_SETTINGS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

/// <summary>
/// Exposes parameters for weather station.
/// </summary>
namespace App_Settings {


	const float BASE_PERIOD_SEC = 1;	// Period to sample anemometer rotations (sec).


	//////////////////const float BASE_PERIOD_SEC = 4;	// Period to sample anemometer rotations (sec).

	/* GPS sync parameters */
	const int	GPS_SATELLITES_REQUIRED = 6;
	const int	GPS_CYCLES_FOR_SYNC = 5;		// Minimum GPS cycles before saving data.
	const int	GPS_DELAY_BETWEEN_CYCLES = 15;	// Delay before getting another GPS fix, sec.
	const float GPS_MAX_ALLOWED_HDOP = 1.5;		// Minimum HDOP precision required before syncing.


	const int	GPS_DUMMY_HOUR = 23;
	const int	GPS_DUMMY_MIN = 55;
	const int	GPS_DUMMY_SEC = 0;
	const int	GPS_DUMMY_DAY = 1;
	const int	GPS_DUMMY_MONTH = 1;
	const int	GPS_DUMMY_YEAR = 2099;

	const int	UTC_OFFSET_HOURS = -8;
	const bool	IS_DAYLIGHT_TIME = true;

	const float WIFI_CONNECT_TIMEOUT_SEC = 5;		// Timeout for connecting to WiFi SSID, sec.
	const float WIFI_CONNECT_TIMEOUT_LOST_SEC = 60;	// Timeout for connecting to WiFi SSID, sec.

	const int FAN_DUTY_PERCENT = 30;			// PWM duty cycle for fan speed.

	const String LOGFILE_PATH_DATA = "/data.txt";

	const String LOGFILE_PATH_STATUS = "/log.txt";

	
	const String LINE_SEPARATOR_LOG_BEGINS = "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";
	const String LINE_SEPARATOR_MAJOR = "===========================================================";
	const String LINE_SEPARATOR = "------------------------------------------------------";

	/*
	ESTIMATE of max. achievable insolation, for
	normalizing insolation percent.
	Value of 2500 for early June, 2022, yielded max. insolation 51%.
	Accidentally used 100mV in August, 2022!!
	Reset to 1500 in Sept., which would have given 85% max.
	in June, close to the summer solstice.
	*/
	const float INSOL_REFERENCE_MAX = 1500;		// Estimated maximum insolation, mV
	const float DAVIS_SPEED_CAL_FACTOR = 2.25;	// WindSpeed calibration factor for Davis anemometer.
	const float VANE_OFFSET = 0;	// Degrees that wind direction reading exceeds true North.
	const float WIND_DIRECTION_SPEED_THRESHOLD = 1;	// WindSpeed below which wind direction is not reported.

	const int LOOP_TIME_WARNING_THRESHOLD_MS = 2000;

	const unsigned int SECONDS_PER_MINUTE = 60;
	const unsigned int MINUTES_PER_HOUR = 60;
	const unsigned int MILLISEC_PER_SECOND = 1000;
	const unsigned long MICROSEC_PER_SECOND = 1000000;
	const unsigned int HOURS_PER_DAY = 24;
	const unsigned int SECONDS_PER_HOUR = SECONDS_PER_MINUTE * MINUTES_PER_HOUR;				// 3,600 s
	const unsigned long SECONDS_PER_DAY = SECONDS_PER_MINUTE * MINUTES_PER_HOUR * HOURS_PER_DAY;// 86,400 s
	const unsigned long MILLISECONDS_PER_HOUR = MILLISEC_PER_SECOND * SECONDS_PER_HOUR;			// 3,600,000 ms
	const unsigned long MILLISECONDS_PER_MINUTE = MILLISEC_PER_SECOND * SECONDS_PER_MINUTE;		// 60,000 ms
	const float DEGREES_PER_RADIAN = 57.2957795130823;	// Degrees in 1 radian.

	/// <summary>
	/// Max size of data lists.
	/// </summary>
	enum speedListSize {
		SIZE_RAW_LIST = 30,		// AT LEAST 30 (for 2-min avg)!
		SIZE_10_MIN_LIST = 24,	// AT LEAST 6 (for 60-min avg)!
		SIZE_60_MIN_LIST = 24,	// AT LEAST 12 (for 12-hr avg)!
		SIZE_DAY_LIST = 30		// Hold data for 30 days.
	};

	/// <summary>
	/// The number of (4.0 sec) base periods in a report interval.
	/// </summary>
	enum BasePeriodsInInterval {
		BASE_PERIODS_IN_10_MIN = 150,	//   600 sec
		BASE_PERIODS_IN_60_MIN = 900,	//	3600 sec
		BASE_PERIODS_IN_24_HR = 21600	// 86400 sec
	};

	/// <summary>
	/// Enumerates data charts requested from the web server.
	/// </summary>
	enum chartRequested {
		CHART_NONE,
		CHART_WIND_SPEED,
		CHART_WIND_GUST,
		CHART_WIND_DIRECTION,
		CHART_TEMPERATURE_F,
		CHART_PRESSURE_SEA_LEVEL,
		CHART_RELATIVE_HUMIDITY,
		CHART_UV_INDEX,
		CHART_INSOLATION,
		CHART_IR_SKY
	};
}

#endif
