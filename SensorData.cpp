// 
// 
// 

#include "SensorData.h"

/// <summary>
/// Exposes methods to read and process sensor data.
/// </summary>
SensorData::SensorData() {
}

/// <summary>
/// Adds (time, value) dataPoint, accumulates average, 
/// and processes min, max.
/// </summary>
/// <param name="dp">(time, value) dataPoint.</param>
void SensorData::addReading(dataPoint dp) {
	_dataLastAdded = dp;	// save most recent
	_countReadings++;
	_sumReadings += dp.value;
	// Smooth the data and find min, max.

	// REPLACE process_Smoothed_Min_Max(dp) with
	// Find min and max so far for today.
	_min_today = (dp.value < _min_today.value) ? dp : _min_today;
	_max_today = (dp.value > _max_today.value) ? dp : _max_today;
	// Find min and max so far for this 10-min period.
	_min_10_min = (dp.value < _min_10_min.value) ? dp : _min_10_min;
	_max_10_min = (dp.value < _max_10_min.value) ? dp : _max_10_min;
	
}


/// <summary>
/// Clears running average and min, max for 10-min period.
/// </summary>
void SensorData::clear_10_min() {
	_sumReadings = 0;
	_countReadings = 0;
	// Reset to highest possible.
	_min_10_min = dataPoint(0, VAL_LIMIT);
	_max_10_min = dataPoint(0, -VAL_LIMIT);
}

/// <summary>
/// Clears saved minimum and maximum for the day.
/// </summary>
void SensorData::clearMinMax_day() {
	// Reset to highest possible.
	_min_today = dataPoint(0, VAL_LIMIT);
	_max_today = dataPoint(0, -VAL_LIMIT);
}

dataPoint SensorData::min_10_min()
{
	return _min_10_min;
}

dataPoint SensorData::max_10_min()
{
	return _max_10_min;
}

/// <summary>
/// Saves data to 10-min list.
/// </summary>
void SensorData::process_data_10_min() {
	// Avg over last 10 min.
	_avg_10_min = _sumReadings / _countReadings;
	// Add to 10-min list of observations.
	addToList(_data_10_min,
		dataPoint(_dataLastAdded.time, _avg_10_min),
		SIZE_10_MIN_LIST);
	clear_10_min();	// Start another 10-min period.
}

/// <summary>
/// Saves data to 60-min list.
/// </summary>
/// <returns></returns>
void SensorData::process_data_60_min() {
	// Average last 6 x 10 min and add to 60-min list.
	_avg_60_min = listAverage(_data_10_min, 6);	// Save latest average.
	addToList(_data_60_min,
		dataPoint(_dataLastAdded.time, _avg_60_min),
		SIZE_60_MIN_LIST);
}

/// <summary>
/// Processes data for a full calendar day.
/// </summary>
/// <returns></returns>
void SensorData::process_data_day() {
	// Save list of daily minima and maxima.
	addToList(_minima_dayList, _min_today, SIZE_DAY_LIST);
	addToList(_maxima_dayList, _max_today, SIZE_DAY_LIST);
	clearMinMax_day();
}

/// <summary>
/// Data point (time, value) of latest sensor reading.
/// </summary>
/// <returns></returns>
dataPoint SensorData::dataLastAdded()
{
	return _dataLastAdded;
}

/// <summary>
/// The most-recently added data value.
/// </summary>
/// <returns></returns>
float SensorData::valueLastAdded()
{
	return _dataLastAdded.value;
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
/// Minimum so far today.
/// </summary>
/// <returns>Today's minimum.</returns>
dataPoint SensorData::min_today()
{
	return _min_today;
}

/// <summary>
/// Maximum so far today.
/// </summary>
/// <returns>Today's maximum.</returns>
dataPoint SensorData::max_today()
{
	return _max_today;
}

/// <summary>
/// List of (time, value) dataPoints at 10-min intervals.
/// </summary>
/// <returns>List of (time, value) dataPoints.</returns>
list<dataPoint> SensorData::data_10_min()
{
	return _data_10_min;
}

/// <summary>
/// List of dataPoints at 60-min intervals.
/// </summary>
/// <returns></returns>
list<dataPoint> SensorData::data_60_min()
{
	return _data_60_min;
}

/// <summary>
/// List of (time, value) dataPoints of daily minima.
/// </summary>
/// <returns>List of (time, value) dataPoints.</returns>
list<dataPoint> SensorData::minima_dayList()
{
	return _minima_dayList;
}

/// <summary>
/// List of (time, value) dataPoints of daily maxima.
/// </summary>
/// <returns>List of (time, value) dataPoints.</returns>
list<dataPoint> SensorData::maxima_dayList()
{
	return _maxima_dayList;
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
	_dataLastAdded = dataPoint(timeStart, valueStart);
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
/// Adds the specified number of elements of dummy data to the 
/// 10-min incrementing the value each time.
/// </summary>
/// <param name="valueStart">Initial value.</param>
/// <param name="increment">Amount to increment the value each time.</param>
/// <param name="numElements">Number of elements to add.</param>
/// <param name="_timeStartLoop">Time assigned to first data point.</param>
void SensorData::addDummyData_maxima(float valueStart,
	float increment,
	int numElements,
	unsigned long timeStart) {
	// Add artificial data to a 60-min list.	
	for (int elem = 0; elem < numElements; elem++)
	{
		dataPoint dp{ timeStart, valueStart };
		addToList(_maxima_dayList, dp, SIZE_60_MIN_LIST);
		valueStart += increment;	// increment value each time.
		timeStart += SECONDS_PER_HOUR;
	}
	dataPoint dpMax{ timeStart, valueStart };
	_max_today = dpMax;
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
/// Returns list of 10-min dataPoints as delimited string.
/// </summary>
/// <returns>List of 10-min dataPoints as delimited string.</returns>
String SensorData::data_10_min_string_delim()
{
	return listToString_dataPoints(_data_10_min);
}

/// <summary>
/// Returns list of 10-min dataPoints as delimited string.
/// </summary>
/// <param name="isConvertZeroToEmpty">
/// Set to true to convert zero to empty string.</param>
/// <param name="decimalPlaces">Decimal places in numbers.</param>
/// <returns>List of 10-min dataPoints as delimited string.</returns>
String SensorData::data_10_min_string_delim(
	bool isConvertZeroToEmpty,
	unsigned int decimalPlaces)
{
	return listToString_dataPoints(_data_10_min,
		isConvertZeroToEmpty,
		decimalPlaces);
}

/// <summary>
/// Returns list of 60-min dataPoints as delimited string.
/// </summary>
/// <returns>List of 60-min dataPoints as delimited string.</returns>
String SensorData::data_60_min_string_delim()
{
	return listToString_dataPoints(_data_60_min);
}

/// <summary>
/// Returns list of 60-min dataPoints as delimited string.
/// </summary>
/// <param name="isConvertZeroToEmpty">Set to true to convert zero to empty string.</param>
/// <param name="decimalPlaces">Decimal places in numbers.</param>
/// <returns>List of 60-min dataPoints as delimited string.</returns>
String SensorData::data_60_min_string_delim(
	bool isConvertZeroToEmpty,
	unsigned int decimalPlaces)
{
	return 	listToString_dataPoints(_data_60_min,
		isConvertZeroToEmpty,
		decimalPlaces);
}

/// <summary>
/// Returns list of maxima dataPoints as delimited string.
/// </summary>
/// <param name="isConvertZeroToEmpty">Set to true to convert zero to empty string.</param>
/// <param name="decimalPlaces">Decimal places in numbers.</param>
/// <returns>List of maxima dataPoints as delimited string.</returns>
String SensorData::maxima_byDay_string_delim(bool isConvertZeroToEmpty, unsigned int decimalPlaces)
{
	return 	listToString_dataPoints(_maxima_dayList,
		isConvertZeroToEmpty,
		decimalPlaces);
}

/// <summary>
/// Returns list of minima dataPoints as delimited string.
/// </summary>
/// <param name="isConvertZeroToEmpty">Set to true to convert zero to empty string.</param>
/// <param name="decimalPlaces">Decimal places in numbers.</param>
/// <returns>List of minima dataPoints as delimited string.</returns>
String SensorData::minima_byDay_string_delim(bool isConvertZeroToEmpty, unsigned int decimalPlaces)
{
	return 	listToString_dataPoints(_minima_dayList,
		isConvertZeroToEmpty,
		decimalPlaces);
}
