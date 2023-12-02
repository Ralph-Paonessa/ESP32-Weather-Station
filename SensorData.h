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
// File system
#include <LittleFS.h>
#include "FileOperations.h"
using namespace FileOperations;

/// <summary>
/// Exposes methods to read and process sensor data.
/// </summary>
class SensorData {

protected:		// Protected items are accessible by inherited classes.

	/// <summary>
	/// Returns a sensor data text-file path.
	/// </summary>
	/// <param name="fileSuffix">Suffix to append to file name.</param>
	String sensorFilepath(String fileSuffix);

	String _label, _labelFile;		// Identifying info.
	String _units, _units_html;		// Units used.

	dataPoint _dataLastAdded;		// Data point (time, value) of most recent reading.

	float _sumReadings;				// Accumulating sum of readings.
	unsigned int _countReadings;		// Number of readings in average.

	// Samples required for smoothing avg.
	const unsigned int COUNT_FOR_SMOOTH = 10;

	float _avg_10_min = 0;			// Average over 10 min.
	float _avg_60_min = 0;			// Average over 60 min.

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

	bool _isDataInFileSys = true;		// Set true to save periodic data in LittleFS file system.
	bool _isReportDailyMaxOnly = false;	// Set true to save maxima but not minima on LittleFS file system.
	bool _isUseSmoothing;				// Set true to smooth data with moving avg and reject outliers.
	float _outlierDelta;				// Factor to determine if reading is an outlier.
	list<float> _avg_moving_List;		// Moving avg of latest reading values.
	float _avgMoving = 0;				// Moving average value.
	unsigned int _avgMoving_Num;		// Maximum number of values to average.

	bool _isConvertZeroToEmpty = true;	//
	unsigned int _decimalPlaces = 0;	//

	bool _isMovingAvgStarted = false;	// Flag to indicate first cycle.


	bool isOutlier(dataPoint dp);

	list<dataPoint> _data_10_min;		// List of Data_Points at 10-min intervals.
	list<dataPoint> _data_60_min;		// List of Data_Points at 60-min intervals.
	list<dataPoint> _minima_dayList;	// List of daily minima.
	list<dataPoint> _maxima_dayList;	// List of daily maxima.

public:

	// Constructor.

	/// <summary>
	/// Creates SensorData instance that exposes 
	/// methods to read and process sensor data.
	/// </summary>
	/// <param name="isDataInFileSys">
	/// Set true to store data in LittleFS file system.</param>
	/// <param name="isReportDailyMaxOnly">
	/// Set true to maxima but not minima LittleFS file system.</param>
	/// <param name="isUseMovingAvg">
	/// Set true to smooth data.</param>
	/// <param name="numSmoothPoints">
	/// Number of points in moving avg.</param>
	/// <param name="outlierDelta">
	/// Range applied to moving avg for outlier rejection.</param>
	SensorData(bool isDataInFileSys = true, 
		bool isReportDailyMaxOnly = false,
		bool isUseMovingAvg = true, 
		unsigned int numSmoothPoints = 5, 
		float outlierDelta = 1.75);

	/// <summary>
	/// Creates files that hold sensor data points at various intervals.
	/// </summary>
	/// <param name="isConvertZeroToEmpty">
	/// Set to true to convert zero to empty in output strings.</param>
	/// <param name="decimalPlaces">Decimal places in output strings.</param>
	void createFiles(bool isConvertZeroToEmpty = true, unsigned int decimalPlaces = 0);

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
	/// The accumulated avg now (reset every 10 minutes)> When 
	/// data smoothing is enabled, outlier values are excluded.
	/// </summary>
	/// <returns>Average now.</returns>
	float avg_now();

	/// <summary>
	/// Returns moving average of last several reading values.
	/// </summary>
	/// <returns>Moving average.</returns>
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

	void addLabels(String label, String labelFile, String units);

	void addLabels(String label, String labelFile, String units, String units_html);

	/// <summary>
	/// Returns display label for the data.
	/// </summary>
	/// <returns>Display label for the data.</returns>
	String label();

	/// <summary>
	/// Returns string for constructing data file name.
	/// </summary>
	/// <returns>String for constructing data file name.</returns>
	String labelFile();


	String units();
	String units_html();

	/// <summary>
	/// Returns list of 10-min dataPoints as delimited string.
	/// </summary>
	/// <returns>List of 10-min dataPoints as delimited string.</returns>
	String data_10_min_string_delim();



	void get_data_10_min_fromFile_DEBUG();

	/// <summary>
	/// Returns delimited String of 10-min data from file.
	/// </summary>
	/// <returns>Delimited String of 10-min data</returns>
	String dataFile_10_min_string_delim();


	String dataFile_60_min_string_delim();


	String dataFile_max_min_string_delim();

	/// <summary>
	/// Returns list of 60-min dataPoints as delimited string.
	/// </summary>
	/// <returns>List of 60-min dataPoints as delimited string.</returns>
	String data_60_min_string_delim();

	/// <summary>
	/// Returns list of 60-min dataPoints as delimited string.
	/// </summary>
	/// <returns>Delimited string of two (time, value) lists, separated by "|".</returns>
	String data_max_min_string_delim();

	/// <summary>
	/// Returns list of daily maxima dataPoints as delimited string.
	/// </summary>
	String maxima_byDay_string_delim();

	/// <summary>
	/// Returns list of daily minima dataPoints as delimited string.
	/// </summary>
	String minima_byDay_string_delim();

	/******     DATA RECOVERY     ******/

	/// <summary>
	/// List of (time, value) dataPoints at 10-min intervals recovered from storage.
	/// </summary>
	/// <returns>List of (time, value) dataPoints recovered from storage.</returns>
	list<dataPoint> recovered_data_10_min();

	/// <summary>
	/// List of (time, value) dataPoints at 60-min intervals, recovered from storage.
	/// </summary>
	/// <returns>List of (time, value) dataPoints recovered from storage.</returns>
	list<dataPoint> recovered_data_60_min();

	/// <summary>
	/// List of (time, value) dataPoints at daily intervals, recovered from storage.
	/// </summary>
	/// <returns>List of (time, value) dataPoints recovered from storage.</returns>
	list<dataPoint> recovered_data_day_min();



	/******     DUMMY DATA     ******/

	void addDummyData_10_min(float valueStart, float increment, int numElements, unsigned long timeStart);

	void addDummyData_60_min(float valueStart, float increment, int numElements, unsigned long timeStart);

	/// <summary>
	/// Adds the specified number of elements of dummy data to the 
	/// daily maxima list, incrementing the value each time.
	/// </summary>
	/// <param name="valueStart">Initial value.</param>
	/// <param name="increment">Amount to increment the value each time.</param>
	/// <param name="numElements">Number of elements to add.</param>
	/// <param name="_timeStartLoop">Time assigned to first data point.</param>
	void addDummyData_maxima_daily(
		float valueStart,
		float increment,
		int numElements,
		unsigned long timeStart);

	/// <summary>
	/// Adds the specified number of elements of dummy data to the 
	/// daily minima list, incrementing the value each time.
	/// </summary>
	/// <param name="valueStart">Initial value.</param>
	/// <param name="increment">Amount to increment the value each time.</param>
	/// <param name="numElements">Number of elements to add.</param>
	/// <param name="_timeStartLoop">Time assigned to first data point.</param>
	void addDummyData_minima_daily(
		float valueStart,
		float increment,
		int numElements,
		unsigned long timeStart);
};
#endif