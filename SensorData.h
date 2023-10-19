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

	dataPoint _dataLastAdded;		// Data point (time, value) of most recent reading.

	float _sumReadings;				// Accumulating sum of readings.
	unsigned int _countReadings;		// Number of readings in average.

	// Samples required for smoothing avg.
	const unsigned int COUNT_FOR_SMOOTH = 10;

	float _avg_10_min = 0;			// Average over 10 min.
	float _avg_60_min = 0;			// Average over 60 min.

	//float _avgSmoothed = 0;			// Avg of the last few readings (for smoothing).

	const float VAL_LIMIT = 999999;	// No reading absolute value will ever be greater.

	/// <summary>
	/// Updates saved min and max values for 10-min period and today.
	/// </summary>
	/// <param name="dp">Data point with value to evaluate.</param>
	void updateMinMax(dataPoint dp);

	// Initialize at impossible extremes.

	dataPoint _min_today = dataPoint(0, VAL_LIMIT);		// Today's minimum.
	dataPoint _max_today = dataPoint(0, -VAL_LIMIT);	// Today's maximum.

	/// <summary>
	/// Minimum sensor reading in the current 10-min period.
	/// </summary>
	dataPoint _min_10_min = dataPoint(0, VAL_LIMIT);	// Initialize at high extreme.

	/// <summary>
	/// Maximum sensor reading in the current 10-min period.
	/// </summary>
	dataPoint _max_10_min = dataPoint(0, -VAL_LIMIT);	// Initialize at low extreme.

	/// <summary>
	/// Clears running average and min, max for 10-min period.
	/// </summary>
	void clear_10_min();

	bool _isUseSmoothing;				// Set true to smooth data with moving avg and reject outliers.
	float _outlierDelta;				// Factor to determine if reading is an outlier.
	list<float> _avg_moving_List;		// Moving avg of latest reading values.
	float _avgMoving = 0;				// Moving average value.
	unsigned int _avgMoving_Num;		// Maximum number of values to average.

	bool _isMovingAvgStarted = false;	// Flag to indicate first cycle.

	
	bool isOutlier(dataPoint dp);

	list<dataPoint> _data_10_min;		// List of Data_Points at 10-min intervals.
	list<dataPoint> _data_60_min;		// List of Data_Points at 60-min intervals.
	list<dataPoint> _minima_dayList;	// List of daily minima.
	list<dataPoint> _maxima_dayList;	// List of daily maxima.

public:

	// Constructor.
	
	/// <summary>
	/// Initializes SensorData instance that exposes 
	/// methods to read and process sensor data.
	/// </summary>
	/// <param name="isUseMovingAvg">
	/// Set true to smooth data.</param>
	/// <param name="numSmoothPoints">
	/// Number of points in moving avg.</param>
	/// <param name="outlierDelta">
	/// Range applied to moving avg for outlier rejection.</param>
	SensorData(
		bool isUseMovingAvg = true, 
		unsigned int numSmoothPoints = 5, 
		float outlierDelta = 1.75
	);

	/// <summary>
	/// Adds (time, value) dataPoint, accumulates average, 
	/// and processes min, max.
	/// </summary>
	/// <param name="dp">(time, value) dataPoint.</param>
	void addReading(dataPoint dp);

	void process_data_10_min();

	void process_data_60_min();

	void process_data_day();

	dataPoint dataLastAdded();

	/// <summary>
	/// The most-recently added data value.
	/// </summary>
	/// <returns>Most recent reading value.</returns>
	float valueLastAdded();

	/// <summary>
	/// The accumulated avg now (reset every 10 minutes).
	/// </summary>
	/// <returns>Average now.</returns>
	float avg_now();


	float avgMoving();

	/// <summary>
	/// The last average saved to the 10-min list.
	/// </summary>
	/// <returns>The last average saved to the 10-min list.
	/// </returns>
	float avg_10_min();

	/// <summary>
	/// The last average saved to the 60-min list.
	/// </summary>
	/// <returns>The last average saved to the 60-min list.
	/// </returns>
	float avg_60_min();

	/// <summary>
	/// Clears saved minimum and maximum for today.
	/// </summary>
	void clearMinMax_day();

	/// <summary>
	/// Returns a (time, value) data point containing the 
	/// minimum sensor reading in the current 10-min 
	/// period. Updates at every reading and resets when new 
	/// 10-min period starts.
	/// </summary>
	/// <returns>Data point with (time, value) minimum 
	/// reading in current 10-min period.</returns>
	dataPoint min_10_min();	// Minimum over 10-min period.

	/// <summary>
	/// Returns a (time, value) data point containing the 
	/// maximum sensor reading in the current 10-min 
	/// period. Updates at every reading and resets when new 
	/// 10-min period starts.
	/// </summary>
	/// <returns>Data point with (time, value) maximum 
	/// reading in current 10-min period.</returns>
	dataPoint max_10_min();// Maximum over 10-min period.

	/// <summary>
	/// Returns a (time, value) data point containing the 
	/// minimum sensor reading today. Updates at 
	/// every reading and resets when day rolls over.
	/// </summary>
	/// <returns>Data point with (time, value) of today's 
	/// minimum reading.</returns>
	dataPoint min_today();

	/// <summary>
	/// Returns a (time, value) data point containing the 
	/// maximum sensor reading today. Updates at 
	/// every reading and resets when day rolls over.
	/// </summary>
	/// <returns>Data point with (time, value) of today's 
	/// maximum reading.</returns>
	dataPoint max_today();

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
	list<dataPoint> minima_dayList();

	/// <summary>
	/// List of (time, value) dataPoints of daily maxima.
	/// </summary>
	/// <returns>List of (time, value) dataPoints.</returns>
	list<dataPoint> maxima_dayList();

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
	String maxima_byDay_string_delim(bool isConvertZeroToEmpty, unsigned int decimalPlaces);

	/// <summary>
	/// Returns list of minima dataPoints as delimited string.
	/// </summary>
	/// <param name="isConvertZeroToEmpty">Set to true to convert zero to empty string.</param>
	/// <param name="decimalPlaces">Decimal places in numbers.</param>
	/// <returns>List of minima dataPoints as delimited string.</returns>
	String minima_byDay_string_delim(bool isConvertZeroToEmpty, unsigned int decimalPlaces);


	void addDummyData_10_min(float valueStart, float increment, int numElements, unsigned long timeStart);

	void addDummyData_60_min(float valueStart, float increment, int numElements, unsigned long timeStart);

	void addDummyData_maxima(
		float valueStart,
		float increment,
		int numElements,
		unsigned long timeStart);	
};

#endif