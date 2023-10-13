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

/// <summary>
/// Adds anemometer reading.
/// </summary>
/// <param name="time">Reading time, sec.</param>
/// <param name="rotations">Anemometer rotation count over BASE_PERIOD.</param>
void WindSpeed::addReading(unsigned long time, int rotations)
{	
	/*
	 Get raw (instantaneous) speed data for this reading. This data
	 will be compiled into averages for various periods (2-, 10-,
	 and 60-min), and these averages will be stored in lists.
	 Should happen at every raw BASE_PERIOD.
	 */
	float speed = speedInstant(rotations, BASE_PERIOD_SEC);	// WindSpeed for BASE_PERIOD.
	_dataLastAdded = dataPoint(time, speed);
	_countReadings++;
	_sumReadings += speed;
	process_Smoothed_Min_Max(_dataLastAdded);
}

/// <summary>
/// Reset accumulate min and max.
/// </summary>
void WindSpeed::clearMinMax_today() {
	_speedMax_10_min = 0;
	_speedMin_10_min = MIN_SPEED_LIMIT;
}

/// <summary>
/// Calculate speed average for 10-min period and hold in list.
/// </summary>
void WindSpeed::process_gusts_10_min() {
	addGust_10_min(_gusts_10_min,
		_speedMax_10_min,
		_speedMin_10_min);
	clearAverage();	// Start another 10-min cycle.	
	clearMinMax_today();
}

/// <summary>
/// Calculate speed average for 60-min period and hold in list.
/// </summary>
void WindSpeed::process_gusts_60_min() {
	_gust_last_60_min = listMaximum(_gusts_10_min, 6);
	addToList(_gusts_60_min, _gust_last_60_min, SIZE_60_MIN_LIST);
}

/// <summary>
/// Processes data for a full calendar day.
/// </summary>
/// <returns></returns>
void WindSpeed::process_gusts_day() {
	// Save list of daily minima and maxima.
	addToList(_minima_dayList,     ?      // _min_today, SIZE_DAY_LIST);
	addToList(_maxima_dayList,     ?      // _max_today, SIZE_DAY_LIST);
	clearMinMax_day();
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
	addToList(targetList, dataPoint(_dataLastAdded.time, gust), SIZE_10_MIN_LIST);
}

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

/// <summary>
/// Returns list of gust (time, value) data points 
/// at 10-min intervals.
/// </summary>
/// <returns>List of gust (time, value) data points.</returns>
list<dataPoint> WindSpeed::gusts_10_min() {
	return _gusts_10_min;
}

/// <summary>
/// Returns list of gust (time, value) data points 
/// at 60-min intervals.
/// </summary>
/// <returns>List of gust (time, value) data points.</returns>
list<dataPoint> WindSpeed::gusts_60_min() {
	return _gusts_60_min;
}

/****  GUST LIST AS CSV STRING  ****/

/// <summary>
/// Returns comma-separated list of gusts for 10-min intervals.
/// </summary>
/// <param name="isConvertZeroToEmpty">
/// Set to true to convert values of zero to empty strings.</param>
/// <param name="decimalPlaces">Decimal places to display.</param>
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
/// <param name="decimalPlaces">Decimal places to display.</param>
/// <returns></returns>
String WindSpeed::gusts_60_min_string_delim(bool isConvertZeroToEmpty, int decimalPlaces)
{
	return listToString_dataPoints(_gusts_60_min, isConvertZeroToEmpty, decimalPlaces);
}

/// <summary>
/// Returns wind speed description in Beaufort 
/// wind strength scale.
/// </summary>
/// <param name="speed">Beaufort wind strength.</param>
/// <returns></returns>
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
