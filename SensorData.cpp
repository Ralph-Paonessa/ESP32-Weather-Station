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
/// Adds dataPoint, accumulates average, and checks for min, max.
/// </summary>
/// <param name="time">Reading time, sec.</param>
/// <param name="value">Reading value.</param>
void SensorData::addReading(unsigned long time, float value) {
	_timeLastRead = time;
	_dataLastRead = dataPoint(time, value);	// save most recent
	_countRead++;
	_sumReadings += value;
	// Smooth the data and find min, max.
	process_Smoothing_MinMax(time, value);	
}

/// <summary>
/// Saves this data point (time, value) as the 
/// new minimum if the value is lower than the 
/// current minimum.
/// </summary>
/// <param name="time">Reading time.</param>
/// <param name="value">Reading value.</param>
dataPoint SensorData::min_Find(const unsigned long& time, const float& value) {
	if (!_isSingleValForMinMax) {
		// Continue comparisons.
		if (value < _minDP.value) {
			return dataPoint(time, value);
		}
	}
	else {
		// Nothing to compare, so start new min.
		return dataPoint(time, value);
	}
}

/// <summary>
/// Saves this data point (time, value) as the 
/// new maximum if the value is higher than the 
/// current maximum.
/// </summary>
/// <param name="time">Reading time.</param>
/// <param name="value">Reading value.</param>
/// <returns>Data point with greater value.<returns>
dataPoint SensorData::max_Find(const unsigned long& time, const float& value) {
	if (!_isSingleValForMinMax) {
		// Continue comparisons.
		if (value > _maxDP.value) {
			return dataPoint(time, value);
		}
	}
	else {
		// Nothing to compare, so start new min and max.
		return dataPoint(time, value);
	}
}

/// <summary>
/// Smooths the data over several cycles 
/// and uses this for min, max.
/// </summary>
/// <param name="time"></param>
/// <param name="value"></param>
void SensorData::process_Smoothing_MinMax(unsigned long time, float value)
{
	_countSmoothRead++;
	if (_countSmoothRead > 1) {
		// There is more than one reading to compare.
		_isSingleValForMinMax = false;
	}
	_sumSmooth += value;
	// Average the values over several cycles to 
	// smooth for min, max.
	if (_countSmoothRead = COUNT_FOR_SMOOTH) {
		// Enough values, so use average for finding min, max.
		float valSmoothed = _sumSmooth / _countSmoothRead;
		_minDP = min_Find(time, valSmoothed);
		_maxDP = max_Find(time, valSmoothed);
		// Restart smoothing.
		clearAverageSmooth();
		_isSingleValForMinMax = true;	// Reset.
	}
}

/// <summary>
/// Clears running average.
/// </summary>
void SensorData::clearAverage() {
	_sumReadings = 0;
	_countRead = 0;
}

/// <summary>
/// Clears smoothing average.
/// </summary>
void SensorData::clearAverageSmooth() {
	_sumSmooth = 0;
	_countSmoothRead = 0;
	_maxDP = dataPoint(0, VAL_LIMIT);	// start at low limit
	_minDP = dataPoint(0, -VAL_LIMIT);	// start at high limit
}

/// <summary>
/// Reset min and max values.
/// </summary>
void SensorData::clearMinMax(){
	_minDP = VAL_LIMIT;		// readings never higher
	_maxDP = -VAL_LIMIT;	// readings never lower	
}

/// <summary>
/// Saves data to 10-min list.
/// </summary>
/// <returns></returns>
void SensorData::process_data_10_min(){
	// Avg over last 10 min.
	_avg_10_min = _sumReadings / _countRead;
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
void SensorData::process_data_60_min(){
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
	addToList(_minima_daily, _minDP, SIZE_DAY_LIST);
	addToList(_maxima_daily, _maxDP, SIZE_DAY_LIST);
	clearMinMax();
	_isSingleValForMinMax = true;
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
/// <returns>Today's minimum.</returns>
dataPoint SensorData::min() {
	return _minDP;
}

/// <summary>
/// Current daily maximum.
/// </summary>
/// <returns>Today's maximum.</returns>
dataPoint SensorData::max() {
	return _maxDP;
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
/// List of dataPoint at 60-min intervals.
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
list<dataPoint> SensorData::minima_daily() {
	return _minima_daily;
}

/// <summary>
/// List of (time, value) dataPoints of daily maxima.
/// </summary>
/// <returns>List of (time, value) dataPoints.</returns>
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
		addToList(_maxima_daily, dp, SIZE_60_MIN_LIST);
		valueStart += increment;	// increment value each time.
		timeStart += SECONDS_PER_HOUR;
	}
	dataPoint dpMax{ timeStart, valueStart };
	_maxDP = dpMax;
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
String SensorData::maxima_daily_string_delim(bool isConvertZeroToEmpty, unsigned int decimalPlaces)
{
	return 	listToString_dataPoints(_maxima_daily,
		isConvertZeroToEmpty,
		decimalPlaces);
}

/// <summary>
/// Returns list of minima dataPoints as delimited string.
/// </summary>
/// <param name="isConvertZeroToEmpty">Set to true to convert zero to empty string.</param>
/// <param name="decimalPlaces">Decimal places in numbers.</param>
/// <returns>List of minima dataPoints as delimited string.</returns>
String SensorData::minima_daily_string_delim(bool isConvertZeroToEmpty, unsigned int decimalPlaces)
{
	return 	listToString_dataPoints(_minima_daily,
		isConvertZeroToEmpty,
		decimalPlaces);
}