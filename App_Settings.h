// App_Settings.h

//--------------------------------------------------------------------
// v12.0 created 3/4/2022
//
//	-- Replace wind cose with routines from WindSpeed.h 
//		custom library.
//--------------------------------------------------------------------
// v11.8 created 2/11/2022
//
//  -- Migrate to Visual vMicro.
//	-- Create custom library WindSpeed.h.
//		Add code at end to test WindSpeed.h by inserting test 
//		functions into setup() and halting all execution after tests.
//--------------------------------------------------------------------
// v11.6 created 1/24/2022
//
//  -- Need to create routines to hold and report recent readings.
//--------------------------------------------------------------------
// v11.5 created 1/23/2022
//
//  -- Add routine to check WiFi and reconnect if lost.
//  -- Clean up code.
//--------------------------------------------------------------------
// v11.4 created 7/2/2001
//
//  -- Add control for PWM fan by copying v1.1_PWM_fan.
//--------------------------------------------------------------------
// v11.3 created 7/1/2021
//
//  -- Add flags for connected sensors.
//  -- Rename anemometer interrupt variables by appending _anem, to 
//		distinguish from others.
//--------------------------------------------------------------------
//--------------------------------------------------------------------
// v11.2 created 6/30/2021 to debug various issues when testing new 
//	PCB 4.2
//
// ---NOTES---
//      **  Added pull-up resistors on board, but they are also on 
//			module boards!!!  **
//      **  This code tries to control GPS power via MOSFET - but 
//			it is NO LONGER ON PCB!!!!!  **
//--------------------------------------------------------------------
// ESP32 Weather Station ver 11.1
//    - Rev 11.1 - Add SPIFFS web server 9/25/2020
//    - Read all sensors.
//    - Log data to SD card.

// Note: Compile problem went away when I changed IDE preferences:
//  Set Compiler warnings to Default. Using More or All
//  resulted in an compile error for the ESP32 board.

// Wrapped all output stings in F() macro, to reduce memory usage.
// Previous memory-like issue actually caused by not wrapping
// one TimLib hour() value as String(hour()), which produced
// corrupted output. It may still be useful to limit the use
// of String for memory reasons?

// This version has sensor logging to SD.
//--------------------------------------------------------------------

// Ralph Paonessa. 8/27/2020

// Hardware interrupt to count anemometer rotations.
// Timer interrupt to control time for rotation count.

//  GOALS  //////////////////////

//    1. Integrate "instantaneous" wind speed over a specified time of
//			rotations. E.g., 2.5 sec.
//    2. Calculate average wind speed over a specified time. E.g., 10 min.
//    3. Determine wind gusts and variance during the wind averaging time.
//    4. Log or report all sensor data after a specified number of wind 
//			speed measurements.

// Standard to follow
//    - measure raw windSpeed over BASE_PERIOD 2.5 sec. (This will not be 
//			reported.)
//    - Get maxWindSpeed as max of windSpeed over 10 min.
//    - Get avgWindSpeed over 1 min.
//    - Get maxWindSpeed over 1 min.
//    - report speed as moving average of avgWindSpeed over a reporting 
//			time of 10 min.
//    - report wind gust as max wind speed during current reporting time.

/*
=================   SENSORS and MODULES   ========================

READING				DEVICE				INTERFACE		CLASS
================	================	============	==========
Temperature			Dallas DS18B20      One-Wire		SensorData
Pressure, RH		Bosch BME280        I2C (1)			SensorData
UV A/B, Index		Vishay VEML6075     I2C (2)			SensorData
IR sky temp.		Melexis MLX90614    I2C (3)			SensorData
Wind speed			Davis Anemometer    HW interrupt	WindSpeed
Wind direction		Davis Wind Vane     ADC voltage		WindDirection
Solar radiation		PV cell             ADC voltage		SensorData
GPS data			u-blox NEO-6M       UART2			GPSModule
SD card module		generic             SPI				SDCard
*/


/***********************************************************
*
 NOTE: isGetReadings (flag set by wind timer) triggers:
	1. Calculate "instantaneous" wind speed from current
		anem_RotationCount and reset anemometer rotation
		counter.
	2. Calculate fan speed and reset fan rotation counter.
	3. Reset isGetReadings (which will change at the next
		anemometer timer interrupt).
	4. Update the wind speed moving average.

*FOR SIMPLICITY, ALL THE OTHER SENSOR READINGS ARE LOGGED
 AT THIS TIME.

 NOTE: Using isGetReadings for BOTH anemometer counts and
 fan counts. This ignores the flag isGetFanRPM and the fan
 hardware timer. (This is done to avoid possibility of
 counting over different times for each. ??? )
***********************************************************/

//////////////////////////////////////////////////////////////////////////////////
/*
WindSpeed wind object gives access to
	- Current speed avg for
		- 2min
		- 10 min
		- 60 min
	- Current gusts for
		- 10 min
		- 60 min
	- speed lists at 2-, 10-, and 60-min intervals
	- gust lists at 10- and 60-min intervals

	HOW DO WE WANT TO USE THIS DATA???

	1. Logging data to SD card at defined intervals. (Every 10 min??)
	2. Current readings for web display on demand.
	3. Providing trend reports over time on demand.

	Ver 11.8 implementation definined globabl variables for
	all readings, and updated these every 60 sec. These values
	were LOGGED TO SD and SERVED BY WEB SERVER.

	A 10-min running average was also tallied and logged, served.

	TIMER INTERRUPT every 2.5 sec triggers raw reading.
	2-MINUTE AVERAGE calculated every 120 sec.
	10-MINUTE AVERAGE calculated every 600 sec.
*/
//////////////////////////////////////////////////////////////////////////////////

/************************************************

PLAN FOR READING AND REPORTING ALL SENSORS

DATA READINGS			2-min	10-min	60-min
----------------------	------	------	------
1. Wind speed & gusts	x		x		x
2. Wind direction				x		x
3. Temperature					x		x
4. Pressure						x		x
5. %RH							x		x
6. Insolation					x		x
7. UV-A							x		x
8. UV-B							x		x
9. UV-Index						x		x
10. IR sky						x		x
11. Fan speed					x		x

Log to SD card every 10 minutes.
*************************************************/

/*
* Where is timing controlled???
*
* Wind has special requirements. BUT, IS TIMING SPECIAL?!?!?!
*
* Main program can keep track of raw periods, and pass them to TimeMonitor
* to identify 2-, 10-, and 6-min periods.
*
* Can use is_N_min() to trigger required operations at those times:
*
*  - Adding averages to lists.
*/



// WindSpeed.h
// v 1.1
// Ralph Paonessa
// 3/9/2022

/*
Timer interrupt signals when to get "raw" wind speed,
which is average over brief BASE_PERIOD, which is shortest
period we can effectively measure. These small periods
are necessary to look for brief gusts.

  GOALS  //////////////////////

	1. Get "instantaneous" wind speed over a specified time of
		rotations (BASE_PERIOD, e.g., 2.5 sec).
	2. Calculate average wind speed over a specified time. E.g., 10 min.
	3. Determine wind gusts and variance during the wind averaging time.
	4. Log or report all sensor data after a specified number of wind
			speed measurements.

 Standard to follow
	- measure raw speeds over BASE_PERIOD 2-5 sec. (This will not be
			public.)
	- Get maxWindSpeed as max of speed over 10 min.
	- Get avgWindSpeed over 2 min.
	- Get maxWindSpeed over 2 min.
	- report speed as average of avgWindSpeed over a reporting
			time of 10 min.
	- report wind gust as max wind speed during current reporting time.


	HOW WIND SPEED IS MEASURED [Ref. 1]

	Defines: "WIND SPEED", "WIND GUST", "SUSTAINED WIND", "PEAK WIND"

	"WIND SPEED"
	Average speed for the most recent 2 minute time. This is considered
	the "SUSTAINED WIND."

	The "WIND SPEED" reported in each dp is an average speed for
	the most recent two-minute time prior to the dp time. This
	is also considered the "SUSTAINED WIND" for routine surface
	observations. This two minute average is calculated	from a series of
	24 five-second average values.

	A "WIND GUST" is also reported when the PEAK "INSTANTANEOUS WIND"
	during the most recent ten-minutes prior to the dp is more
	than 10 knots greater than the lowest "lull" in the wind during that
	time. If that is the case, the highest INSTANTANEOUS WIND during that
	ten minute window is reported as the GUST value.

	If the MAXIMUM INSTANTANEOUS WIND SPEED during the entire time since
	the last reported dp exceeds 25 knots (28.77 mph), that value
	is reported	as the "PEAK WIND." It will be at least equal to the
	reported gust,and may be higher than the reported gust if the "peak"
	occurred more than 10 minutes prior to the dp time.

	-----------------------------------------------------------

	WIND GUSTS [Ref. 2]

	According to U.S. weather observing practice, gusts are reported when the
	"PEAK WIND SPEED"(*) reaches at least 16 knots (18.41 mph) and the variation
	in wind speed between the peaks and lulls is at least 9 knots (10.36 mph).
	The duration of a gust is usually less than 20 seconds.

	(*)NOTE: "PEAK WIND SPEED" [2] <---> PEAK "INSTANTANEOUS WIND" [1]

________________

	[2] https://graphical.weather.gov/definitions/defineWindGust.html

**  MY TAKE FROM [1] et al.  **

There is some ambiguity or flexibility regarding how often to average
and report a wind speed. But it seems that the shortest practical period
might be 2 minutes. However, "instantaneous" wind speed measurements
are necessary to determine gusts.

Because it is impractical to maintain records of the speed at 2-min
intervals, it is up to the user to decide what frequency to save/report. For
the NWS, this appears to be 10-min or 60-min intervals, with the latter
being what is actually saved as archival weather data.

In our case, we wish to present recent data to users as graphs over the
following intervals:
	10-min
	60-min
	12-hr (which can be delimited as DAY and NIGHT)

These can be used to report trends, such as daily maximum and minimum.

2-min intervals are used internally, but not reported to the user.


1. Measure wind over short BASE_PERIOD (2-5 sec) --> "RAW SPEED"

2. Average RAW SPEEDs over 2-min period --> "SUSTAINED WIND" aka "REPORTED
	WIND SPEED" aka "2-MIN-AVG"

3. WIND GUST determined from PEAK INSTANTANEOUS WIND aka RAW WIND SPEED.

4.


HOW TO IMPLEMENT:

1. Measure wind over short BASE_PERIOD (2-5 sec) --> (speedInstant). Check for MAX (_speedMax_10_min).

2. Average RAW SPEED over 2-min period -->  2-MIN-AVG  (SUSTAINED WIND aka REPORTED WIND SPEED)

3. 10-min interval
	Get average_10_min from _speeds_2_min list.
	Get gust as max during last 10-min (_speedMax_10_min).

3. Find max every 10-min, save as gust

3. Calc 10-min avg    average_10_min from last five _speeds_2_min[]

4. 60-min, 12-hr intervals
	Avg and add to lists.



	IMPLEMENTATION
	INST_SPEED = anemometer output over 2.5 seconds. (Not reported.)
	SPEED = *2 minute* average OF INST_SPEED   (48 cycles x 2.5 sec).
	GUST: Occurs if (MAX Inst. WindSpeed) during most recent 10 minute
	time is at least 10 knots greater than (MIN Inst. WindSpeed) during time.

	If (MAX - MIN) >= 10, then GUST = MAX.   // over 10 min time

	- Report readings every *2 min*
	- Accumulate values over *10 min*
	- Look at past *10 min* to get gusts
	- Summarize data every *1hour*
	- Maintain data in memory for last N hours, to show trends.



 Data will be logged every ( countPeriod * numForAvgWindSpeed ) sec.
 Not worthwhile to report "instantaneous" wind speed measured every
 few seconds.
	- Clutters up data.
	- WindSpeed can be highly variable, therefore misleading.
 Wind is averaged over ( countPeriod * numForAvgWindSpeed ) seconds.
 Some other data should be averaged, such as wind directionCardinal, strength.

 When someone requests data, they get the last logged data, which is
 at most X min old?
 Can also include most recent measurements?
*/

/*

What is optimum wind speed sampling frequency?
	Too low		-->	Lowest speeds inaccurate.
					But, miss brief high gusts.
	Too high	--> Excess processing required.

----------------------------------------------------
SAMPLE WIND SPEED MEASUREMENT VALUES
----------------------------------------------------
WindSpeed	Freq.		Counts		Counts		Counts
mph		Hz			in 0.25s	in 2.5s		in 120s
-----	------		--------	--------	--------
1		0.44		0.1			1.1			53
2		0.89		0.2			2.2			107
3		1.33		0.3			3.3			160
4		1.78		0.4			4.4			213
5		2.22		0.6			5.6			267
6		2.67		0.7			6.7			320
7		3.11		0.8			7.8			373
8		3.56		0.9			8.9			427
9		4.00		1.0			10.0		480
10		4.44		1.1			11.1		533
11		4.89		1.2			12.2		587
12		5.33		1.3			13.3		640
13		5.78		1.4			14.4		693
14		6.22		1.6			15.6		747
15		6.67		1.7			16.7		800
20		8.89		2.2			22.2		1067
30		13.3		3.3			33.3		1600
40		17.8		4.4			44.4		2133
50		22.2		5.6			55.6		2667
60		26.7		6.7			66.7		3200
70		31.1		7.8			77.8		3733
80		35.6		8.9			88.9		4267
90		40.0		10.0		100.0		4800
100		44.4		11.1		111.1		5333
125		55.6		13.9		138.9		6667
150		66.7		16.7		166.7		8000
175		77.8		19.4		194.4		9333
200		88.9		22.2		222.2		10667
---------------------------------------------------
*/

/*
NOTE: We can compile the various average_N values into
lists that can be called by the program.

The calling program MUST HANDLE THE TIMING of processing to
produce these lists (BASE_PERIOD, 2-min, 10-min, ... ) by calling
	addReading()
	process_data_2_min()
	process_data_10_min()
	...

The calling program will call these lists when it:
	- Losg data to SD and or serial monitor at N-minute intervals.
	- Populates a web request with data.

For each N-min time, must
	- Calculate N-min avg
	- Add avg to list_N
	- Determine gust (max) in time

	NOTE: Handling GUSTS is what makes wind different from other
	sensor data. It requires examing the INSTANTANEOUS speeds over
	short time periods, and looking for maxima.



10/18/2022

SPEED
	For current data reporting, use 10-min avg.

	Can keep averaging speed for 10 minutes, save this, then reset.

	For every instantaneous reading, hold min and max (for gust calc).

GUSTS
	For current data reporting, use max over 10-min.

	Find gust when max exceeds avg (and max exceeds min by threshold).

	Save gust every 10 min, and reset max, min.


*/

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

	const float BASE_PERIOD = 4;	// Period to sample anemometer rotations (sec).

	/* GPS sync parameters */
	const int	GPS_SATELLITES_REQUIRED = 3;
	const int	GPS_CYCLES_FOR_SYNC = 4;		// Minimum GPS cycles before saving data.
	const int	GPS_DELAY_BETWEEN_CYCLES = 10;	// Delay before getting another GPS fix, sec.
	const float GPS_MAX_ALLOWED_HDOP = 2.5;		// Minimum HDOP precision required before syncing.

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
