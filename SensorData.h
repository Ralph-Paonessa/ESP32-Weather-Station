// SensorData.h

#ifndef _SENSORDATA_h
#define _SENSORDATA_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <list>
using std::list;

#include "dataPoint.h"
#include "ListFunctions.h"
#include "App_settings.h"
using namespace ListFunctions;
using namespace App_Settings;

/// <summary>
/// Exposes methods to read and process sensor data.
/// </summary>
class SensorData {

protected:		// Protected items are accessible by inherited classes.

	String _label, _labelShort;		// Identifying info.
	String _units, _units_html;		// Units used.

	long _timeLastRead;				// Time of most recent reading.
	dataPoint _dataLastRead;		// Data point (time, value) of most recent reading.

	float _sumReadings;				// Accumulating sum of readings.
	unsigned int _countRead;		// Number of readings in average.

	float _sumSmooth;
	unsigned int _countSmoothRead = 0;

	// Samples required for smoothing avg.
	const unsigned int COUNT_FOR_SMOOTH = 10;

	float _avg_10_min = 0;			// Average over 10 min.
	float _avg_60_min = 0;			// Average over 60 min.

	void process_Smoothing_MinMax(unsigned long time, float value);

	float _avgSmoothed = 0;			// Avg of the last few readings (for smoothing).
	list<float> _valuesToSmooth;	// List of recent values to average for smoothing.

	const float VAL_LIMIT = 999999;	// No sensor value will ever be greater.

	dataPoint _minDP = dataPoint(0, -VAL_LIMIT);	// dp (time, min val). 
	dataPoint _maxDP = dataPoint(0, VAL_LIMIT);		// dp (time, max val). 
		
	/// <summary>
	/// Clears running average.
	/// </summary>
	void clearAverage();

	/// <summary>
	/// Clears smoothing average.
	/// </summary>
	void clearAverageSmooth();

	/// <summary>
	/// Clears saved minimum and maximum data points.
	/// </summary>
	void clearMinMax();

	// True if only a single value so far for min, max.
	bool _isSingleValForMinMax = true;

	list<dataPoint> _data_10_min;	// List of Data_Points at 10-min intervals.
	list<dataPoint> _data_60_min;	// List of Data_Points at 60-min intervals.
	list<dataPoint> _minima_daily;	// List of daily minima.
	list<dataPoint> _maxima_daily;	// List of daily maxima.

public:
	// Constructor.
	SensorData();

	/// <summary>
	/// Adds dataPoint, accumulates average, and checks for min, max.
	/// </summary>
	/// <param name="time">Reading time, sec.</param>
	/// <param name="value">Reading value.</param>
	void addReading(unsigned long time, float value);


	/// <summary>
	/// Saves this data point (time, value) as the 
	/// new minimum if the value is lower than the 
	/// current minimum.
	/// </summary>
	/// <param name="time">Reading time.</param>
	/// <param name="value">Reading value.</param>
	dataPoint min_Find(const unsigned long& time, const float& value);

	/// <summary>
	/// Saves this data point (time, value) as the 
	/// new maximum if the value is higher than the 
	/// current maximum.
	/// </summary>
	/// <param name="time">Reading time.</param>
	/// <param name="value">Reading value.</param>
	/// </summary>
	/// <returns>Data point with greater value.<returns>
	dataPoint max_Find(const unsigned long& time, const float& value);

	void process_data_10_min();
	void process_data_60_min();

	void process_data_day();

	float valueLastAdded();
	float avg_10_min();
	float avg_60_min();


	/// <summary>
	/// Returns dataPoint with today's minimum value so far.
	/// </summary>
	/// <returns>dataPoint (time, min_value)</returns>
	dataPoint min();

	/// <summary>
	/// Returns dataPoint with today's maximum value so far.
	/// </summary>
	/// <returns>dataPoint (time, max_value)</returns>
	dataPoint max();

	/// <summary>
	/// List of (time, value) dataPoints at 10-min intervals.
	/// </summary>
	/// <returns>List of (time, value) dataPoints.</returns>
	list<dataPoint> data_10_min();

	/// <summary>
	/// List of (time, value) dataPoints at 60-min intervals.
	/// </summary>
	/// <returns>List of (time, value) dataPoints.</returns>
	list<dataPoint> data_60_min();

	/// <summary>
	/// List of (time, value) dataPoints of daily minima.
	/// </summary>
	/// <returns>List of (time, value) dataPoints.</returns>
	list<dataPoint> minima_daily();

	/// <summary>
	/// List of (time, value) dataPoints of daily maxima.
	/// </summary>
	/// <returns>List of (time, value) dataPoints.</returns>
	list<dataPoint> maxima_daily();




	void addLabels(String label, String labelShort, String units);
	void addLabels(String label, String labelShort, String units, String units_html);

	String label();
	String labelShort();
	String units();
	String units_html();


	String data_10_min_string_delim();

	/// <summary>
	/// Returns list of 10-min dataPoints as delimited string.
	/// </summary>
	/// <param name="isConvertZeroToEmpty">
	/// Set to true to convert zero to empty string.</param>
	/// <param name="decimalPlaces">Decimal places in numbers.</param>
	/// <returns>List of 10-min dataPoints as delimited string.</returns>
	String data_10_min_string_delim(bool isConvertZeroToEmpty, unsigned int decimalPlaces);


	String data_60_min_string_delim();

	/// <summary>
	/// Returns list of 60-min dataPoints as delimited string.
	/// </summary>
	/// <param name="isConvertZeroToEmpty">
	/// Set to true to convert zero to empty string.</param>
	/// <param name="decimalPlaces">Decimal places in numbers.</param>
	/// <returns>List of 60-min dataPoints as delimited string.</returns>
	String data_60_min_string_delim(bool isConvertZeroToEmpty, unsigned int decimalPlaces);


	/// <summary>
	/// Returns list of maxima dataPoints as delimited string.
	/// </summary>
	/// <param name="isConvertZeroToEmpty">Set to true to convert zero to empty string.</param>
	/// <param name="decimalPlaces">Decimal places in numbers.</param>
	/// <returns>List of maxima dataPoints as delimited string.</returns>
	String maxima_daily_string_delim(bool isConvertZeroToEmpty, unsigned int decimalPlaces);

	/// <summary>
	/// Returns list of minima dataPoints as delimited string.
	/// </summary>
	/// <param name="isConvertZeroToEmpty">Set to true to convert zero to empty string.</param>
	/// <param name="decimalPlaces">Decimal places in numbers.</param>
	/// <returns>List of minima dataPoints as delimited string.</returns>
	String minima_daily_string_delim(bool isConvertZeroToEmpty, unsigned int decimalPlaces);


	void addDummyData_10_min(float valueStart, float increment, int numElements, unsigned long timeStart);
	void addDummyData_60_min(float valueStart, float increment, int numElements, unsigned long timeStart);


	void addDummyData_maxima(
		float valueStart,
		float increment,
		int numElements,
		unsigned long timeStart);


};

#endif