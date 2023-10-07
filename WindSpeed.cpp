// WindSpeed.h
// 
// Ralph Paonessa
// 

#include "WindSpeed.h"

/// <summary>
/// Initializes WindSpeed object.
/// </summary>
/// <param name="calibrationFactor"></param>
WindSpeed::WindSpeed(float calibrationFactor)
{
	_calibrationFactor = calibrationFactor;
}

///// <summary>
///// Adds anemometer reading and returns instantaneous speed.
///// </summary>
///// <param name="time">Reading time, sec.</param>
///// <param name="rotations">Anemometer rotation count over BASE_PERIOD.</param>
//float WindSpeed::addReading(unsigned long time, int rotations)
//{
//	_timeLastRead = time;
//	/*
//	 Get raw (instantaneous) speed data for this reading. This data
//	 will be compiled into averages for various periods (2-, 10-,
//	 and 60-min), and these averages will be stored in lists.
//	 Should happen at every raw BASE_PERIOD.
//	 */
//	float speed = speedInstant(rotations, BASE_PERIOD);		// WindSpeed for BASE_PERIOD.
//	_timeLastRead = time;
//	_speedLastAdded = speed;
//	_countReadings++;
//	_sumReadings += speed;
//	_speedMax_10_min = maxValue(speed, _speedMax_10_min);	// Highest in last 10 min period.
//	_speedMin_10_min = minValue(speed, _speedMin_10_min);	// Lowest in last 10 min period.
//	return speed;
//}
//
///// <summary>
///// Clears accumulated speed and gust readings.
///// </summary>
//void WindSpeed::clearAverage()
//{
//	_sumReadings = 0;
//	_countReadings = 0;
//}

/// <summary>
/// Reset accumulate min and max.
/// </summary>
void WindSpeed::clearMinMax() {
	_speedMax_10_min = 0;
	_speedMin_10_min = MIN_SPEED_LIMIT;
}

///// <summary>
///// Calculate speed average for 10-min period and hold in list.
///// </summary>
//void WindSpeed::process_data_10_min() {
//	// Average readings so far (should be last 10 min).	
//	float avg = _sumReadings / _countReadings;
//	// Add to the 10-min list.
//	addToList(_data_10_min,
//		dataPoint(_timeLastRead, avg),
//		SIZE_10_MIN_LIST);
//	_speed_last_10_min = avg;
//	// Record any wind gusts in past 10 min.
//	addGust_10_min(_gusts_10_min,
//		_speedMax_10_min,
//		_speedMin_10_min);
//	clear();	// Start another 10-min cycle.
//}




/// <summary>
/// Calculate speed average for 10-min period and hold in list.
/// </summary>
void WindSpeed::process_gusts_10_min() {
	//// Average readings so far (should be last 10 min).	
	//float avg = _sumReadings / _countReadings;
	//// Add to the 10-min list.
	//addToList(_data_10_min,
	//	dataPoint(_timeLastRead, avg),
	//	SIZE_10_MIN_LIST);
	//_speed_last_10_min = avg;
	// Record any wind gusts in past 10 min.
	addGust_10_min(_gusts_10_min,
		_speedMax_10_min,
		_speedMin_10_min);
	clearAverage();	// Start another 10-min cycle.	
	clearMinMax();
	// XXX XXX XXX DATA WAS ALREADY CLEARED BY process_data_10_min() !!!!!!!!!!!!!
	// FIX BY USING MULTIPLE clear() ?????

}


///// <summary>
///// Calculate speed average for 60-min period and hold in list.
///// </summary>
//void WindSpeed::process_data_60_min() {
//	// Average last 6 speeds in 10-min list.	
//	float avg = listAverage(_data_10_min, 6);	// 1-hr avg.
//	addToList(_data_60_min, dataPoint(_timeLastRead, avg), SIZE_60_MIN_LIST);
//	// Get 60-min gust as maximum gust from last 6 x 10-min gusts.
//	_gust_last_60_min = listMaximum(_gusts_10_min, 6);
//	addToList(_gusts_60_min, _gust_last_60_min, SIZE_60_MIN_LIST);
//}

/// <summary>
/// Calculate speed average for 60-min period and hold in list.
/// </summary>
void WindSpeed::process_gust_60_min() {
	//// Average last 6 speeds in 10-min list.	
	//float avg = listAverage(_data_10_min, 6);	// 1-hr avg.
	//addToList(_data_60_min, dataPoint(_timeLastRead, avg), SIZE_60_MIN_LIST);
	// Get 60-min gust as maximum gust from last 6 x 10-min gusts.
	_gust_last_60_min = listMaximum(_gusts_10_min, 6);
	addToList(_gusts_60_min, _gust_last_60_min, SIZE_60_MIN_LIST);
}

/// <summary>
/// Returns wind speed from anemometer rotations.
/// </summary>
/// <param name="rotations"> Number of rotations.</param>
/// <param name="period">Time period of rotations, sec.</param>
/// <returns>Wind speed, mph</returns>
float WindSpeed::speedInstant(int rotations, float period) {
	/*************************************************************
	Davis anemometer formula:
		speed = rotations * 2.25 / time		[from Davis spec].
			where
			time = time during which rotations are counted (sec).
	*************************************************************/
	return rotations * _calibrationFactor / period;
}

/// <summary>
/// Checks for any wind speed (over 10-min time) that meets gust
/// criteria and adds to list. (Adds 0 if no gust found.)
/// </summary>
/// <param name="targetList">List to add gust to.</param>
/// <param name="max">Maximum speed so far.</param>
/// <param name="min">Minimum speed so far.</param>
void WindSpeed::addGust_10_min(list<dataPoint>& targetList, float max, float min) {
	float gust = 0;
	// Gust must meet criteria.
	if (
		max >= GUST_THRESHOLD				// must exceed threshold, and
		|| ((max - min) >= GUST_SPREAD)		// must exceed minimum by by GUST_SPREAD
		) {	
		gust = max;							// New gust.
	}
	// Add gust to 10-min gust list.
	_gust_last_10_min = gust;
	addToList(targetList, dataPoint(_timeLastRead, gust), SIZE_10_MIN_LIST);
}

///// <summary>
///// Adds label information to the data.
///// </summary>
///// <param name="label">Label for the data.</param>
///// <param name="labelShort">Brief label for the data.</param>
///// <param name="units">The units of the data.</param>
//void WindSpeed::addLabels(String label, String labelShort, String units)
//{
//	_label = label;
//	_labelShort = labelShort;
//	_units = units;
//}

///// <summary>
///// Adds label information to the data.
///// </summary>
///// <param name="label">Label for the data.</param>
///// <param name="labelShort">Brief label for the data.</param>
///// <param name="units">Data units.</param>
///// <param name="units_html">Data units with html encoding.</param>
//void WindSpeed::addLabels(String label, String labelShort, String units, String units_html)
//{
//	addLabels(label, labelShort, units);
//	_units_html = units_html;
//}

///// <summary>
///// Label for the  data.
///// </summary>
///// <returns>String</returns>
//String WindSpeed::label() {
//	return _label;
//}
//
///// <summary>
///// Abbreviated label for the data.
///// </summary>
///// <returns>String</returns>
//String WindSpeed::labelShort() {
//	return _labelShort;
//}
//
///// <summary>
///// Data units.
///// </summary>
///// <returns>String</returns>
//String WindSpeed::units() {
//	return _units;
//}
//
///// <summary>
///// Sensor units with html encoding.
///// </summary>
///// <returns>String</returns>
//String WindSpeed::units_html() {
//	return _units_html;
//}

///// <summary>
///// Adds the specified number of elements of dummy speed data, 
///// incrementing the value each time.
///// </summary>
///// <param name="valueStart">Initial value.</param>
///// <param name="increment">Amount to increment the value each time.</param>
///// <param name="numElements">Number of elements to add.</param>
///// /// <param name="_timeStartLoop">Time assigned to first data point.</param>
//void WindSpeed::addDummySpeedData_10_min(
//	float valueStart,
//	float increment,
//	int numElements,
//	unsigned long timeStart) {
//	// Add artificial speed data to a 10-min list.	
//	for (int elem = 1; elem < numElements; elem++) {
//		dataPoint dp{ timeStart, valueStart };
//		addToList(_data_10_min, dp, SIZE_10_MIN_LIST);
//		valueStart += increment;	// increment value each time.
//		timeStart += SECONDS_PER_MINUTE * 10;
//	}
//}

/// <summary>
/// Adds the specified number of elements of dummy gust data, 
/// incrementing the value each time.
/// </summary>
/// <param name="valueStart">Initial value.</param>
/// <param name="increment">Amount to increase the value each time.</param>
/// <param name="numElements">Number of elements to add.</param>
/// /// <param name="_timeStartLoop">Time assigned to first data point.</param>
void WindSpeed::addDummyGustData_10_min(
	float valueStart,
	float increment,
	int numElements,
	unsigned long timeStart) {
	// Add artificial speed data to a 10-min list.	
	for (int elem = 1; elem < numElements; elem++)
	{
		dataPoint dp{ timeStart, valueStart };
		addToList(_gusts_10_min, dp, SIZE_10_MIN_LIST);
		valueStart += increment;	// increment value each time.
		timeStart += SECONDS_PER_MINUTE * 10;
	}
}

/////   HELPER FUNCTIONS   /////

///// <summary>
///// Return the maximum of value1 and value2.
///// </summary>
///// <param name="val1"></param>
///// <param name="val2"></param>
///// <returns></returns>
//float WindSpeed::maxValue(float val1, float val2) {
//	if (val1 > val2) {
//		return val1;	// val1 is higher.
//	}
//	else {
//		return val2;	// val2 is higher.
//	}
//}

///// <summary>
///// Return the minimum of value1 and value2.
///// </summary>
///// <param name="val1"></param>
///// <param name="val2"></param>
///// <returns></returns>
//float WindSpeed::minValue(float val1, float val2) {
//	if (val1 < val2) {
//		return val1;	// val1 is lower.
//	}
//	else {
//		return val2;	// val2 is lower.
//	}
//}

/////   RETURN PROPERTIES   /////

///// <summary>
///// List of average wind speeds at 10-min intervals.
///// </summary>
///// <returns></returns>
//list<dataPoint> WindSpeed::data_10_min() {
//	return _data_10_min;
//}

///// <summary>
///// List of average wind speeds at 60-min intervals.
///// </summary>
///// <returns></returns>
//list<dataPoint> WindSpeed::data_60_min() {
//	return _data_60_min;
//}

///// <summary>
///// The most recently added speed, mph.
///// </summary>
///// <returns></returns>
//float WindSpeed::valueLastAdded() {
//	return _speedLastAdded;
//}

///// <summary>
///// Latest 10-min speed average.
///// </summary>
///// <returns>Float</returns>
//float WindSpeed::avg_10_min()
//{
//	return _speed_last_10_min;
//}
//
///// <summary>
///// Latest 60-min speed average.
///// </summary>
///// <returns>Float</returns>
//float WindSpeed::avg_60_min()
//{
//	return _speed_last_60_min;
//}

/// <summary>
/// Latest 10-min maximum gust.
/// </summary>
/// <returns>Float</returns>
float WindSpeed::gust_10_min()
{
	return _gust_last_10_min;
}

/// <summary>
/// Latest 60-min maximum gust.
/// </summary>
/// <returns>Float</returns>
float WindSpeed::gust_60_min()
{
	return _gust_last_60_min;
}

///// <summary>
///// String of (time, value) data points as t1,v1~t2,v2~ ... 
///// </summary>
///// <param name="isConvertZeroToEmpty"></param>
///// <param name="decimalPlaces"></param>
///// <returns></returns>
//String WindSpeed::speeds_10_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces)
//{
//	return listToString_dataPoints(_data_10_min, isConvertZeroToEmpty, decimalPlaces);
//}
//
///// <summary>
///// String of (time, value) data points as t1,v1~,t2,v2~ ... 
///// </summary>
///// <param name="isConvertZeroToEmpty"></param>
///// <param name="decimalPlaces"></param>
///// <returns></returns>
//String WindSpeed::speeds_60_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces)
//{
//	return listToString_dataPoints(_data_60_min, isConvertZeroToEmpty, decimalPlaces);
//}

/// <summary>
/// List of gusts at 10-min intervals.
/// </summary>
/// <returns></returns>
list<dataPoint> WindSpeed::gusts_10_min() {
	return _gusts_10_min;
}

/// <summary>
/// List of gusts at 60-min intervals.
/// </summary>
/// <returns></returns>
list<dataPoint> WindSpeed::gusts_60_min() {
	return _gusts_60_min;
}

/****  GUST LIST AS CSV STRING  ****/

/// <summary>
/// Returns comma-separated list of gusts for 10-min intervals.
/// </summary>
/// <param name="isConvertZeroToEmpty">
/// Set to true to convert values of zero to empty strings.</param>
/// <param name="decimalPlaces">Decimal places to round to.</param>
/// <returns></returns>
String WindSpeed::gusts_10_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces)
{
	return listToString_dataPoints(_gusts_10_min, isConvertZeroToEmpty, decimalPlaces);
}

/// <summary>
/// Returns comma-separated list of gusts for 60-min intervals.
/// </summary>
/// <param name="isConvertZeroToEmpty">
/// Set to true to convert values of zero to empty strings.</param>
/// <param name="decimalPlaces">Decimal places to round to.</param>
/// <returns></returns>
String WindSpeed::gusts_60_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces)
{
	return listToString_dataPoints(_gusts_60_min, isConvertZeroToEmpty, decimalPlaces);
}

// Translate wind speed to wind strength scale, and print.
String WindSpeed::beaufortWind(float speed) {
	if (speed < 1)
		return "Calm";
	else if (speed < 4)
		return "Light Air";
	else if (speed < 8)
		return "Light Breeze";
	else if (speed < 13)
		return "Gentle Breeze";
	else if (speed < 19)
		return "Moderate Breeze";
	else if (speed < 25)
		return "Fresh Breeze";
	else if (speed < 32)
		return "Strong Breeze";
	else if (speed < 39)
		return "High Wind";
	else
		return "RUN";
}

