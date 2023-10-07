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

	//String _label, _labelShort;		// Identifying info
	//String _units, _units_html;		// Units used	
	//unsigned long _timeLastRead;	// last reading time, seconds from 1970

	//float _speedLastAdded;			// Speed last added, mph.

	//float _sumReadings;				// Accumulating sum of speeds from readings.
	//int _countReadings;		// Number of readings since initialization.

	const float GUST_THRESHOLD = 18.41;	// WindSpeed must exceed this to be a gust.
	const float GUST_SPREAD = 10.36;	// WindSpeed must exceed low by this amount to be a gust.
	const float MIN_SPEED_LIMIT = 9999;	// Real minimum will always be lower than this.

	void addGust_10_min(list<dataPoint>& targetList, float max, float min);

	//void clearAverage();	// Override.

	/// <summary>
	/// Reset min and max values.
	/// </summary>
	void clearMinMax();

	//list<dataPoint> _data_10_min;	// List of avg speeds at 10-min intervals.
	//list<dataPoint> _data_60_min;	// List of avg speeds at 60-min intervals.

	//float _speed_last_10_min;	// Latest dataPoint of avg speed over last 10 min.
	//float _speed_last_60_min;	// Latest dataPoint of avg speed over last 60 min.

	float _gust_last_10_min;	// Gust that occurred in last 10 min.
	float _gust_last_60_min;	// Gust that occurred in last 60 min.

	// Variables to hold windSpeed extremes for gusts.

	float _speedMax_10_min = 0;					// Maximum speed in 10-min time.
	float _speedMin_10_min = MIN_SPEED_LIMIT;	// Minimum speed in 10-min time.

	list<dataPoint> _gusts_10_min;	// List of gust data points over 10-min period.
	list<dataPoint> _gusts_60_min;	// List of gust data points over 60-min period.

	//// Compare values.
	//float maxValue(float val, float max);
	//float minValue(float val, float min);

public:

	// Constructor	
	WindSpeed(float calibrationFactor);		// Overload of SensorData.

	//// XXX THIS IS VOID IN BASE! HERE IT RETURNS CALCULATE SPEED. 
	//// NEED NEW PUBLIC PROPERTY??? OR USE speedInstant()???
	//float addReading(unsigned long time, int rotations);

	/// <summary>
	/// Returns wind speed from anemometer rotations.
	/// </summary>
	/// <param name="rotations"> Number of rotations.</param>
	/// <param name="period">Time period of rotations, sec.</param>
	/// <returns>Wind speed, mph</returns>
	float speedInstant(int rotations, float period);

	//void addLabels(String label, String labelShort, String units);
	//void addLabels(String label, String labelShort, String units, String units_html);

	//void process_data_10_min();		// Does more than Sensor Data because of gusts.
	//void process_data_60_min();		// Does more than Sensor Data because of gusts.


	void process_gusts_10_min();
	void process_gust_60_min();

	/*float speedLastAdded();
	float speed_last_10_min();
	float speed_last_60_min();*/

	//float valueLastAdded();

	// XXX REPLACE??
	//float speed_last_10_min();
	//float speed_last_60_min();

	//float avg_10_min();
	//float avg_60_min();

	float gust_10_min();
	float gust_60_min();

	// XXX ARE BOTH NEEDED?
	//void addDummySpeedData_10_min(float valueStart, float increment, int numElements, unsigned long timeStart);
	void addDummyGustData_10_min(float valueStart, float increment, int numElements, unsigned long timeStart);

	/*list<dataPoint> data_10_min();
	list<dataPoint> data_60_min();*/

	//String speeds_10_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces);
	//String speeds_60_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces);

	String gusts_10_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces);
	String gusts_60_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces);

	/*String label();
	String labelShort();
	String units();
	String units_html();*/

	list<dataPoint> gusts_10_min();
	list<dataPoint> gusts_60_min();

	String beaufortWind(float speed);

	/*WindSpeed() = default;
	WindSpeed(float _calibrationFactor, float _speedLastAdded, float _sumReadings, int _countReadings, float GUST_THRESHOLD, float GUST_SPREAD, float MIN_SPEED_LIMIT, float _speed_last_10_min, float _speed_last_60_min, float _gust_last_10_min, float _gust_last_60_min, float _speedMax_10_min, float _speedMin_10_min, const list<dataPoint>& _gusts_10_min, const list<dataPoint>& _gusts_60_min)
		: _calibrationFactor(_calibrationFactor), _speedLastAdded(_speedLastAdded), _sumReadings(_sumReadings), _countReadings(_countReadings), GUST_THRESHOLD(GUST_THRESHOLD), GUST_SPREAD(GUST_SPREAD), MIN_SPEED_LIMIT(MIN_SPEED_LIMIT), _speed_last_10_min(_speed_last_10_min), _speed_last_60_min(_speed_last_60_min), _gust_last_10_min(_gust_last_10_min), _gust_last_60_min(_gust_last_60_min), _speedMax_10_min(_speedMax_10_min), _speedMin_10_min(_speedMin_10_min), _gusts_10_min(_gusts_10_min), _gusts_60_min(_gusts_60_min);*/
};

#endif
