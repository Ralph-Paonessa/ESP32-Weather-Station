// WindSpeed.h
// v 1.1
// Ralph Paonessa
// 3/9/2022

#ifndef _WINDSPEED_h
#define _WINDSPEED_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <list>
using std::list;

#include "ListFunctions.h"
#include "App_settings.h"

using namespace ListFunctions;
using namespace App_Settings;

#include "SensorData.h"

/// <summary>
/// Exposes methods to measure and record wind speeds.
/// </summary>
class WindSpeed : public SensorData {	// Inherits SensorData.

private:
	float _calibrationFactor;		// Wind speed calibration factor for anemometer.

	const float GUST_THRESHOLD = 18.41;	// WindSpeed must exceed this to be a gust.
	const float GUST_SPREAD = 10.36;	// WindSpeed must exceed low by this amount to be a gust.
	const float MIN_SPEED_LIMIT = 9999;	// Real minimum will always be lower than this.

	void addGust_10_min(list<dataPoint>& targetList, float max, float min);

	/// <summary>
	/// Reset min and max values.
	/// </summary>
	void clearMinMax_today();

	dataPoint _min_gust_today = dataPoint(0, VAL_LIMIT);	// Today's minimum.
	dataPoint _max_gust_today = dataPoint(0, -VAL_LIMIT);	// Today's maximum.

	float _gust_last_10_min;	// Gust that occurred in last 10 min.
	float _gust_last_60_min;	// Gust that occurred in last 60 min.

	// Variables to hold windSpeed extremes for gusts.

	float _speedMax_10_min = 0;					// Maximum speed in 10-min time.
	float _speedMin_10_min = MIN_SPEED_LIMIT;	// Minimum speed in 10-min time.

	list<dataPoint> _gusts_10_min;	// List of gust data points over 10-min period.
	list<dataPoint> _gusts_60_min;	// List of gust data points over 60-min period.

public:

	// Constructor	
	WindSpeed(float calibrationFactor);		// Overload of SensorData.

	void addReading(unsigned long time, int rotations);


	/// <summary>
	/// Returns wind speed from anemometer rotations.
	/// </summary>
	/// <param name="rotations"> Number of rotations.</param>
	/// <param name="period">Time period of rotations, sec.</param>
	/// <returns>Wind speed, mph</returns>
	float speedInstant(int rotations, float period);

	void process_gusts_10_min();
	void process_gusts_60_min();

	void process_gusts_day();

	float gust_10_min();
	float gust_60_min();

	void addDummyGustData_10_min(
		float valueStart, 
		float increment, 
		int numElements, 
		unsigned long timeStart);

	String gusts_10_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces);
	String gusts_60_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces);

	/// <summary>
	/// Returns list of gust (time, value) data points 
	/// at 10-min intervals.
	/// </summary>
	/// <returns>List of gust (time, value) data points.</returns>
	list<dataPoint> gusts_10_min();

	/// <summary>
	/// Returns list of gust (time, value) data points 
	/// at 60-min intervals.
	/// </summary>
	/// <returns>List of gust (time, value) data points.</returns>
	list<dataPoint> gusts_60_min();

	/// <summary>
	/// Returns wind speed description in Beaufort 
	/// wind strength scale.
	/// </summary>
	/// <param name="speed">Beaufort wind strength.</param>
	/// <returns></returns>
	String beaufortWind(float speed);

};

#endif
