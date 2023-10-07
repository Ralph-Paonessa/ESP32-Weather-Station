// 
// 
// 

#include "SensorData.h"

/// <summary>
/// Base class to to inherit to record and process sensor data.
/// </summary>
SensorData::SensorData() {
}

/// <summary>
/// Adds dataPoint and accumulates average.
/// </summary>
/// <param name="time">Reading time, sec.</param>
/// <param name="value">Reading value.</param>
void SensorData::addReading(unsigned long time, float value) {
	_timeLastRead = time;
	_dataLastRead = dataPoint(time, value);	// save most recent
	_countReadings++;
	_sumReadings += value;
	min_Find(time, value);
	max_Find(time, value);
	_isMinMaxRestart = false;
}


/// <summary>
/// Saves this data point (time, value) as the 
/// new minimum if the value is lower than the 
/// current minimum.
/// </summary>
/// <param name="time">Reading time.</param>
/// <param name="value">Reading value.</param>
void SensorData::min_Find(const unsigned long& time, const float& value) {
	if (!_isMinMaxRestart) {
		// Continue comparisons.
		if (value < _min.value) {
			_min = dataPoint(time, value);
		}
	}
	else {
		// Nothing to compare, so start new min.
		_min = dataPoint(time, value);
	}
}

/// <summary>
/// Saves this data point (time, value) as the 
/// new maximum if the value is higher than the 
/// current maximum.
/// </summary>
/// <param name="time">Reading time.</param>
/// <param name="value">Reading value.</param>
void SensorData::max_Find(const unsigned long& time, const float& value) {
	if (!_isMinMaxRestart) {
		// Continue comparisons.
		if (value > _max.value) {
			_max = dataPoint(time, value);
		}
	}
	else {
		// Nothing to compare, so start new min and max.
		_max = dataPoint(time, value);
	}
}

/// <summary>
/// Clears running average, but leaves data average lists intact.
/// </summary>
void SensorData::clearAverage() {
	_sumReadings = 0;
	_countReadings = 0;
}

/// <summary>
/// Reset min and max values.
/// </summary>
void SensorData::clearMinMax()
{
	_min = 999999;	// readings never higher
	_max = -999999;	// readings never lower
	_isMinMaxRestart = true;
}

/// <summary>
/// Saves data to 10-min list.
/// </summary>
/// <returns></returns>
void SensorData::process_data_10_min()
{
	// Avg over last 10 min.
	_avg_10_min = _sumReadings / _countReadings;
	// Add to 10-min list of observations.
	addToList(_data_10_min,
		dataPoint(_timeLastRead, _avg_10_min),
		SIZE_10_MIN_LIST);
	clearAverage();	// start another 10-min avg
}

/// <summary>
/// Saves data to 60-min list.
/// </summary>
/// <returns></returns>
void SensorData::process_data_60_min()
{
	// Average last 6 x 10 min and add to 60-min list.
	_avg_60_min = listAverage(_data_10_min, 6);	// Save the latest average.
	addToList(_data_60_min,
		dataPoint(_timeLastRead, _avg_60_min),
		SIZE_60_MIN_LIST);
}

/// <summary>
/// Processes data for a full calendar day.
/// </summary>
/// <returns></returns>
void SensorData::process_data_day() {
	// Save list of daily minima and maxima.
	addToList(_minima_daily, _min, SIZE_DAY_LIST);
	addToList(_maxima_daily, _max, SIZE_DAY_LIST);
	clearMinMax();
}

/// <summary>
/// The most-recently added data value.
/// </summary>
/// <returns></returns>
float SensorData::valueLastAdded()
{
	return _dataLastRead.value;
}

/// <summary>
/// The last average saved to the 10-min list.
/// </summary>
/// <returns></returns>
float SensorData::avg_10_min()
{
	return _avg_10_min;
}

/// <summary>
/// The last average saved to the 60-min list.
/// </summary>
/// <returns></returns>
float SensorData::avg_60_min()
{
	return _avg_60_min;
}


/// <summary>
/// Current daily minimum.
/// </summary>
/// <returns></returns>
dataPoint SensorData::min() {
	return _min;
}


dataPoint SensorData::max() {
	return _max;
}





/// <summary>
/// List of dataPoint at 10-min intervals.
/// </summary>
/// <returns></returns>
list<dataPoint> SensorData::data_10_min()
{
	return _data_10_min;
}

/// <summary>
/// List of dataPoint at 60-min intervals.
/// </summary>
/// <returns></returns>
list<dataPoint> SensorData::data_60_min()
{
	return _data_60_min;
}

/// <summary>
/// List of dataPoints of daily minima.
/// </summary>
/// <returns></returns>
list<dataPoint> SensorData::minima_daily() {
	return _minima_daily;
}

/// <summary>
/// List of dataPoints of daily maxima.
/// </summary>
/// <returns></returns>
list<dataPoint> SensorData::maxima_daily() {
	return _maxima_daily;
}

/// <summary>
/// Adds the specified number of elements of dummy data, 
/// incrementing the value each time. Also copies valueStart
/// to valueLastAdded and avg_10_min.
/// </summary>
/// <param name="valueStart">Initial value.</param>
/// <param name="increment">Amount to increment the value each time.</param>
/// <param name="numElements">Number of elements to add.</param>
/// <param name="_timeStartLoop">Time assigned to first data point.</param>
void SensorData::addDummyData_10_min(float valueStart,
	float increment,
	int numElements,
	unsigned long timeStart) {
	for (int elem = 0; elem < numElements; elem++)
	{
		dataPoint dp{ timeStart, valueStart };
		addToList(_data_10_min, dp, SIZE_10_MIN_LIST);
		valueStart += increment;	// increment value each time.
		timeStart += SECONDS_PER_MINUTE * 10;
	}
	_dataLastRead = dataPoint(timeStart, valueStart);
	_avg_10_min = valueStart;
}

/// <summary>
/// Adds the specified number of elements of dummy data to the 
/// 10-min incrementing the value each time.
/// </summary>
/// <param name="valueStart">Initial value.</param>
/// <param name="increment">Amount to increment the value each time.</param>
/// <param name="numElements">Number of elements to add.</param>
/// <param name="_timeStartLoop">Time assigned to first data point.</param>
void SensorData::addDummyData_60_min(float valueStart,
	float increment,
	int numElements,
	unsigned long timeStart) {
	// Add artificial data to a 60-min list.	
	for (int elem = 0; elem < numElements; elem++)
	{
		dataPoint dp{ timeStart, valueStart };
		addToList(_data_60_min, dp, SIZE_60_MIN_LIST);
		valueStart += increment;	// increment value each time.
		timeStart += SECONDS_PER_HOUR;
	}
	_avg_60_min = valueStart;
}

/// <summary>
/// Adds label information to the data.
/// </summary>
/// <param name="label">Label for the data.</param>
/// <param name="labelShort">Brief label for the data.</param>
/// <param name="units">The units of the data.</param>
void SensorData::addLabels(String label, String labelShort, String units)
{
	_label = label;
	_labelShort = labelShort;
	_units = units;
}

/// <summary>
/// Adds label information to the data.
/// </summary>
/// <param name="label">Label for the data.</param>
/// <param name="labelShort">Brief label for the data.</param>
/// <param name="units">Data units.</param>
/// <param name="units_html">Data units with html encoding.</param>
void SensorData::addLabels(String label,
	String labelShort,
	String units,
	String units_html)
{
	addLabels(label, labelShort, units);
	_units_html = units_html;
}

/// <summary>
/// Label for the  data.
/// </summary>
/// <returns>String</returns>
String SensorData::label()
{
	return _label;
}

/// <summary>
/// Abbreviated label for the data.
/// </summary>
/// <returns>String</returns>
String SensorData::labelShort()
{
	return _labelShort;
}

/// <summary>
/// Data units.
/// </summary>
/// <returns>String</returns>
String SensorData::units()
{
	return _units;
}

/// <summary>
/// Sensor units with html encoding.
/// </summary>
/// <returns>String</returns>
String SensorData::units_html()
{
	return _units_html;
}

/// <summary>
/// Returns list of 10-min dataPoint as delimited string.
/// </summary>
/// <returns>String</returns>
String SensorData::data_10_min_string_delim()
{
	return listToString_dataPoints(_data_10_min);
}

/// <summary>
/// Returns list of 10-min dataPoint as delimited string.
/// </summary>
/// <param name="isConvertZeroToEmpty">Set to true to convert zero to empty string.</param>
/// <param name="decimalPlaces">Decimal places in numbers.</param>
/// <returns></returns>
String SensorData::data_10_min_string_delim(
	bool isConvertZeroToEmpty,
	int decimalPlaces)
{
	return listToString_dataPoints(_data_10_min,
		isConvertZeroToEmpty,
		decimalPlaces);
}

/// <summary>
/// Returns list of 60-min dataPoint as delimited string.
/// </summary>
/// <returns>String</returns>
String SensorData::data_60_min_string_delim()
{
	return listToString_dataPoints(_data_60_min);
}
/// <summary>
/// Returns list of 60-min dataPoint as delimited string.
/// </summary>
/// <param name="isConvertZeroToEmpty">Set to true to convert zero to empty string.</param>
/// <param name="decimalPlaces">Decimal places in numbers.</param>
/// <returns></returns>
String SensorData::data_60_min_string_delim(
	bool isConvertZeroToEmpty,
	int decimalPlaces)
{
	return 	listToString_dataPoints(_data_60_min,
		isConvertZeroToEmpty,
		decimalPlaces);
}

