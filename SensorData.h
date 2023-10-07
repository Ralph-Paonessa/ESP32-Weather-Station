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

// Base class to to inherit to record and process sensor data.
class SensorData {

protected:		// Protected items are accessible by inherited classes.

	String _label, _labelShort;		// Identifying info.
	String _units, _units_html;		// Units used.

	long _timeLastRead;				// Time of most recent reading.
	dataPoint _dataLastRead;		// Data point (time, value) of most recent reading.

	float _sumReadings;				// Accumulating sum of readings.
	int _countReadings;				// Number of readings since initialization.

	float _avg_10_min = 0;			// Average over 10 min.
	float _avg_60_min = 0;			// Average over 60 min.

	dataPoint _min;					// Data point with time and minimum value. 
	dataPoint _max;					// Data point with time and maximum value.

	/// <summary>
	/// Clears running average, but leaves data average lists intact.
	/// </summary>
	void clearAverage();
		
	/// <summary>
	/// Clears saved minimum and maximum data points.
	/// </summary>
	void clearMinMax();

	bool _isMinMaxRestart = true;	// True if no min or max values have been recorded yet.

	list<dataPoint> _data_10_min;	// List of Data_Points at 10-min intervals.
	list<dataPoint> _data_60_min;	// List of Data_Points at 60-min intervals.
	list<dataPoint> _minima_daily;	// List of daily minima.
	list<dataPoint> _maxima_daily;	// List of daily maxima.

public:
	// Constructor.
	SensorData();

	/// <summary>
	/// Adds dataPoint and accumulates average.
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
	void min_Find(const unsigned long& time, const float& value);

	/// <summary>
	/// Saves this data point (time, value) as the 
	/// new maximum if the value is higher than the 
	/// current maximum.
	/// </summary>
	/// <param name="time">Reading time.</param>
	/// <param name="value">Reading value.</param>
	/// </summary>
	/// <param name="time"></param>
	/// <param name="value"></param>
	void max_Find(const unsigned long& time, const float& value);

	void process_data_10_min();
	void process_data_60_min();

	void process_data_day();

	float valueLastAdded();
	float avg_10_min();
	float avg_60_min();


	/// <summary>
	/// Returns dataPoint with minimum value so far.
	/// </summary>
	/// <returns>dataPoint (time, min_value)</returns>
	dataPoint min();

	/// <summary>
	/// Returns dataPoint with maximum value so far.
	/// </summary>
	/// <returns>dataPoint (time, max_value)</returns>
	dataPoint max();

	list<dataPoint> data_10_min();
	list<dataPoint> data_60_min();

	list<dataPoint> minima_daily();
	list<dataPoint> maxima_daily();


	void addDummyData_10_min(float valueStart, float increment, int numElements, unsigned long timeStart);
	void addDummyData_60_min(float valueStart, float increment, int numElements, unsigned long timeStart);


	void addLabels(String label, String labelShort, String units);
	void addLabels(String label, String labelShort, String units, String units_html);

	String label();
	String labelShort();
	String units();
	String units_html();

	String data_10_min_string_delim();
	String data_10_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces);
	String data_60_min_string_delim();
	String data_60_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces);
};


#endif

