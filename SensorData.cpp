// 
// 
// 

#include "SensorData.h"

// Constructor.

/// <summary>
/// Initializes SensorData instance that exposes 
/// methods to read and process sensor data.
/// </summary>
/// <param name="isUseSmoothing">Set true to smooth data.</param>
/// <param name="numInMovingAvg">Number of points in moving avg.</param>
/// <param name="outlierDelta">
/// Range applied to moving avg for outlier rejection.</param>
SensorData::SensorData(
	bool isUseSmoothing, 
	unsigned int numInMovingAvg, 
	float outlierDelta, 
	bool isConvertZeroToEmpty, 
	unsigned int decimalPlaces
) {
	_isUseSmoothing = isUseSmoothing;
	_avgMoving_Num = numInMovingAvg;
	_outlierDelta = outlierDelta;
	_isConvertZeroToEmpty = isConvertZeroToEmpty;
	_decimalPlaces = decimalPlaces;
}

/// <summary>
/// Adds (time, value) dataPoint, accumulates average, 
/// and updates min and max.
/// </summary>
/// <param name="dp">(time, value) dataPoint.</param>
void SensorData::addReading(dataPoint dp) {
	_dataLastAdded = dp;	// save most recent

	/*
	Want outlier detection so that a large wind gust won't "pollute"
	the moving avg wind speed. Outlier will still be reported as the last
	value read and it will still be compared by windGust to the moving
	avg to decide if it's a wind gust.
	*/

	/*
	* XXX
	*
	USING A FACTOR TO DEFINE THE OUTLIER RANGE *SUCKS*
	FOR READINGS NEAR ZERO!!!
	*
	/*
	DANGER:
	On the first pass, _avgMoving = 0, and so the high and low outlier
	comparisons will all be zero! That means that any reading value will
	be ouside the range [0, 0] and will be declared an outlier, and won't
	be added to the moving avg. Thus, the moving avg will always be zero,
	and all values will be outliers!

	Have to do something to break out of this cycle when starting.

	Can skip outlier check the first time through.

		- This will fail if the first value happens to be 0!
	*/


	/*
	NOTE: If this is the first cycle, there's not yet
	a value assigned to _avgMoving nor any data points
	in _avg_moving_List.

		_avgMoving = 0 (as initialized).

	When _avgMoving is zero, the multiples used for the
	outlier range will be [0, 0]. Therefore, any nonzero
	value will be outside this range and marked as an outlier!

	So, NO VALUE will be saved!!!
	*/


	//// First value begins moving avg. (First value CAN'T BE AN OUTLIER!!)
	//if (!_isMovingAvgStarted) {
	//	_avgMoving = dp.value;
	//	_isMovingAvgStarted = true;
	//}

	if (!_isUseSmoothing) {
		// No smoothing. No moving avg.
		_countReadings++;
		_sumReadings += dp.value;
	}
	else
	{
		// Apply SMOOTHING and OUTLIER REJECTION.

		// First value begins moving avg. (First value CAN'T BE AN OUTLIER!!)
		if (!_isMovingAvgStarted) {
			_avgMoving = dp.value;
			_isMovingAvgStarted = true;
		}

		if (!isOutlier(dp))
		{
			// Not an outlier, so include in 10-min avg.
			_countReadings++;
			_sumReadings += dp.value;

			// Not an outlier, so include in moving avg.
			addToList(_avg_moving_List, dp.value, _avgMoving_Num);
			_avgMoving = listAverage(_avg_moving_List, _avgMoving_Num);
			_isMovingAvgStarted = true;
		}
	}

	updateMinMax(dp);	// Regardless of outlier status.
}

/// <summary>
/// Returns true if the data value is outside the limits set 
/// by [_avgMoving / _avgMoving, _avgMoving * _avgMoving].
/// </summary>
/// <param name="dp">Data point with value to evaluate.</param>
/// <returns>True if this is an outlier.</returns>
bool SensorData::isOutlier(dataPoint dp) {
	// NEEDED?? XXX //////////////
	if (_avgMoving == 0) {
		return false;
	}
	///////////////////////////////
	bool isOut = (dp.value > _avgMoving + _avgMoving) || (dp.value < _avgMoving - _outlierDelta);		// lower bound
	return isOut;
}

/// <summary>
/// Updates saved min and max values for 
/// current 10-min period and all of today.
/// </summary>
/// <param name="dp">Data point with value to evaluate.</param>
void SensorData::updateMinMax(dataPoint dp) {
	// Update min and max so far for this 10-min period.
	_min_10_min = (dp.value < _min_10_min.value) ? dp : _min_10_min;
	_max_10_min = (dp.value > _max_10_min.value) ? dp : _max_10_min;

	// Update min and max so far for all of today.
	_min_today = (dp.value < _min_today.value) ? dp : _min_today;
	_max_today = (dp.value > _max_today.value) ? dp : _max_today;
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
/// Clears saved minimum and maximum for today.
/// </summary>
void SensorData::clearMinMax_day() {
	// Reset to highest possible.
	_min_today = dataPoint(0, VAL_LIMIT);
	_max_today = dataPoint(0, -VAL_LIMIT);
}

/// <summary>
/// Returns a (time, value) data point containing the 
/// minimum sensor reading in the current 10-min 
/// period. Updates at every reading and resets when new 
/// 10-min period starts.
/// </summary>
/// <returns>Data point with (time, value) minimum 
/// reading in current 10-min period.</returns>
dataPoint SensorData::min_10_min()
{
	return _min_10_min;
}

/// <summary>
/// Returns a (time, value) data point containing the 
/// maximum sensor reading in the current 10-min 
/// period. Updates at every reading and resets when new 
/// 10-min period starts.
/// </summary>
/// <returns>Data point with (time, value) maximum 
/// reading in current 10-min period.</returns>
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

	// Store in LittleFS
	String path = "/Sensor readings/" + _label + ".txt";
	appendFile(LittleFS, path.c_str(), data_10_min_string_delim().c_str());

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
/// <returns>Most recent reading value.</returns>
float SensorData::valueLastAdded()
{
	return _dataLastAdded.value;
}

/// <summary>
/// The accumulated avg now (reset every 10 minutes) when data smoothing is enabled, outlier values are not included in this average.
/// </summary>
/// <returns>Average now.</returns>
float SensorData::avg_now()
{
	return _sumReadings / _countReadings;
}

/// <summary>
/// Returns moving average of last several reading values.
/// </summary>
/// <returns>Moving average.</returns>
float SensorData::avgMoving()
{
	return _avgMoving;
}

/// <summary>
/// The last average saved to the 10-min list.
/// </summary>
/// <returns>10-min average.</returns>
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
/// Returns a (time, value) data point containing the 
/// minimum sensor reading today. Updates at 
/// every reading and resets when day rolls over.
/// </summary>
/// <returns>Data point with (time, value) of today's 
/// minimum reading.</returns>
dataPoint SensorData::min_today()
{
	return _min_today;
}

/// <summary>
/// Returns a (time, value) data point containing the 
/// maximum sensor reading today. Updates at 
/// every reading and resets when day rolls over.
/// </summary>
/// <returns>Data point with (time, value) of today's 
/// maximum reading.</returns>
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
		timeStart += SECONDS_PER_MINUTE * 10;	// 10-min interval
	}
	_dataLastAdded = dataPoint(timeStart, valueStart);
	_avg_10_min = valueStart;
}

/// <summary>
/// Adds the specified number of elements of dummy data to the 
/// 10-min list, incrementing the value each time.
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
		valueStart += increment;		// increment value each time.
		timeStart += SECONDS_PER_HOUR;	// 1-hr interval
	}
	_avg_60_min = valueStart;
}

/// <summary>
/// Adds the specified number of elements of dummy data to the 
/// daily maxima list, incrementing the value each time.
/// </summary>
/// <param name="valueStart">Initial value.</param>
/// <param name="increment">Amount to increment the value each time.</param>
/// <param name="numElements">Number of elements to add.</param>
/// <param name="_timeStartLoop">Time assigned to first data point.</param>
void SensorData::addDummyData_maxima_daily(
	float valueStart,
	float increment,
	int numElements,
	unsigned long timeStart) {
	// Add artificial data to a 60-min list.	
	for (int elem = 0; elem < numElements; elem++)
	{
		dataPoint dp{ timeStart, valueStart };
		addToList(_maxima_dayList, dp, SIZE_60_MIN_LIST);
		valueStart += increment;		// increment value each time.
		timeStart += SECONDS_PER_DAY;	// 1-day interval
	}
	dataPoint dpMax{ timeStart, valueStart };
	_max_today = dpMax;
}

/// <summary>
/// Adds the specified number of elements of dummy data to the 
/// daily minima list, incrementing the value each time.
/// </summary>
/// <param name="valueStart">Initial value.</param>
/// <param name="increment">Amount to increment the value each time.</param>
/// <param name="numElements">Number of elements to add.</param>
/// <param name="_timeStartLoop">Time assigned to first data point.</param>
void SensorData::addDummyData_minima_daily(
	float valueStart,
	float increment,
	int numElements,
	unsigned long timeStart) {
	// Add artificial data to a 60-min list.	
	for (int elem = 0; elem < numElements; elem++)
	{
		dataPoint dp{ timeStart, valueStart };
		addToList(_minima_dayList, dp, SIZE_60_MIN_LIST);
		valueStart += increment;		// increment value each time.
		timeStart += SECONDS_PER_DAY;	// 1-day interval
	}
	dataPoint dpMax{ timeStart, valueStart };
	_min_today = dpMax;
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
	return listToString_dataPoints(_data_10_min,
		_isConvertZeroToEmpty,
		_decimalPlaces);
}


/// <summary>
/// Returns list of 60-min dataPoints as delimited string.
/// </summary>
/// <returns>List of 60-min dataPoints as delimited string.</returns>
String SensorData::data_60_min_string_delim()
{
	return 	listToString_dataPoints(_data_60_min,
		_isConvertZeroToEmpty,
		_decimalPlaces);
}


/// <summary>
/// Returns combined dlimited lists of list of daily 
/// maxima and minima dataPoints,delimited by "|"string.
/// </summary>
/// <returns>Delimited string of two (time, value) lists, separated by "|".</returns>
String SensorData::data_max_min_string_delim()
{
	return 	listToString_dataPoints(_maxima_dayList, 
		_minima_dayList, 
		_isConvertZeroToEmpty, 
		_decimalPlaces);
}






/// <summary>
/// Returns list of daily maxima dataPoints as delimited string.
/// </summary>
/// <returns>List of maxima dataPoints as delimited string.</returns>
String SensorData::maxima_byDay_string_delim()
{
	return 	listToString_dataPoints(_maxima_dayList,
		_isConvertZeroToEmpty,
		_decimalPlaces);
}

/// <summary>
/// Returns list of minima dataPoints as delimited string.
/// </summary>
/// <returns>List of minima dataPoints as delimited string.</returns>
String SensorData::minima_byDay_string_delim()
{
	return 	listToString_dataPoints(_minima_dayList,
		_isConvertZeroToEmpty,
		_decimalPlaces);
}
