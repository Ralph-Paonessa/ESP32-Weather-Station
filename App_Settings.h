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

	///////const float BASE_PERIOD_SEC = 1;	// Period to sample anemometer rotations (sec).

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

	const float BASE_PERIOD_SEC = 4;	// Period to sample anemometer rotations (sec).

	const unsigned int DATA_RECOVERY_10_MIN_CUTOFF = 30 * SECONDS_PER_MINUTE;	// Recover 10-min data no older.
	const unsigned int DATA_RECOVERY_60_MIN_CUTOFF = 3 * SECONDS_PER_HOUR;		// Recover 60-min data no older.
	const unsigned int DATA_RECOVERY_DAY_CUTOFF = 3 * SECONDS_PER_DAY;			// Recover day data no older.


	/* GPS sync parameters */

	const unsigned long GPS_BAUD_RATE = 9600;		// NEO-6M 9600

	const unsigned int	GPS_SATELLITES_REQUIRED = 3;// Minimum number of GPS satellites before syncing.
	const unsigned int	GPS_CYCLES_FOR_SYNC = 3;	// Minimum GPS cycles before syncing.
	const unsigned int	GPS_CYCLE_DELAY_SEC = 2;	// Delay before getting another GPS fix, sec.
	const unsigned int	GPS_CYCLES_COUNT_MAX = 50;	// Max number of GPS cycles before quitting.
	const float GPS_MAX_ALLOWED_HDOP = 4;			// Minimum HDOP precision for syncing.

	const unsigned int	GPS_DUMMY_HOUR = 23;		// Hour for dummy GPS time.
	const unsigned int	GPS_DUMMY_MIN = 05;			// Minute for dummy GPS time.
	const unsigned int	GPS_DUMMY_SEC = 0;			// Second for dummy GPS time.
	const unsigned int	GPS_DUMMY_DAY = 1;			// Day for dummy GPS time.
	const unsigned int	GPS_DUMMY_MONTH = 1;		// Month for dummy GPS time.
	const unsigned int	GPS_DUMMY_YEAR = 2099;		// Year for dummy GPS time.

	const int	UTC_OFFSET_HOURS = -8;
	const bool	IS_DAYLIGHT_TIME = true;

	const float WIFI_CONNECT_TIMEOUT_SEC = 5;		// Timeout for connecting to WiFi SSID, sec.
	const float WIFI_CONNECT_TIMEOUT_LOST_SEC = 60;	// Timeout for connecting to WiFi SSID, sec.

	const unsigned int FAN_DUTY_PERCENT = 30;		// PWM duty cycle for fan speed.

	const String SENSOR_DATA_DIR_PATH = "/Sensor data";	// Absolute path to sensor data files directory.
	const String SENSOR_DATA_TIME_FILE_PATH = "/Sensor data/last_time.txt";	// Absolute path to sensor read time file.


	//char CHAR_GLOBAL_BUFFER[2048] = {  };			// Globally-defined character array buffer.


	const String LOGFILE_PATH_DATA = "/data.txt";

	const String LOGFILE_PATH_STATUS = "/log.txt";

	const String LINE_SEPARATOR_LOG_BEGINS = "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";
	const String LINE_SEPARATOR_MAJOR = "=====================================================";
	const String LINE_SEPARATOR = "-------------------------------------------------";

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
	const float WIND_SPEED_OUTLIER_DELTA = 10;
	const unsigned int WIND_SPEED_NUMBER_IN_MOVING_AVG = 5;


	const unsigned int LOOP_TIME_WARNING_THRESHOLD_MS = 2000;

	/// <summary>
	/// Enumerate lists of sensor data of different periods.
	/// </summary>
	enum dataList {
		LIST_10_MIN = 0,
		LIST_60_MIN = 1,
		LIST_DAY = 2
	};

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