// 
// 
// 

#include "WindSpeed2.h"

/// <summary>
/// Initializes WindSpeed2 object.
/// </summary>
/// <param name="calibrationFactor">
/// Calibration factor for anemometer model.</param>
WindSpeed2::WindSpeed2(float calibrationFactor)
{
	_calibrationFactor = calibrationFactor;
}

/// <summary>
/// Returns wind speed from anemometer rotations.
/// </summary>
/// <param name="rotations"> Number of rotations.</param>
/// <param name="period">Time period of rotations, sec.</param>
/// <returns>Wind speed, mph</returns>
float WindSpeed2::speedInstant(int rotations, float period)
{
	/*************************************************************
	Davis anemometer formula:
		speed = rotations * 2.25 / time		[from Davis spec].
			where
			time = time during which rotations are counted (sec).
	*************************************************************/
	return rotations * _calibrationFactor / period;
}

/// <summary>
/// Checks for and returns a gust datPoint if the speed satisfies 
/// gust criteria.Otherwise, the returned value will be zero if 
/// the speed doesn't satisfy gust criteria.
/// </summary>
/// <param name="speed">(time, value) point to evaluate for gust.</param>
/// <returns>(time, value) data point.</returns>
dataPoint WindSpeed2::gust(dataPoint speed)
{
	// Gust must meet criteria.
	if (
		// Gust exceeds threshold
		speed.value >= GUST_THRESHOLD
		&&
		// Gust exceeds minimum by GUST_SPREAD
		((speed.value - _min_10_min.value) >= GUST_SPREAD)	
		)
	{
		// Found a gust.
		return speed;
	}
	else
	{
		// Not a gust, so return value of zero.
		return dataPoint(speed.time, 0);
	}
}

///// <summary>
///// Checks for any wind speed (over 10-min time) that meets gust
///// criteria and adds to list. (Adds 0 if no gust found.)
///// </summary>
///// <param name="targetList">List to add gust to.</param>
///// <param name="max">Maximum speed so far.</param>
///// <param name="min">Minimum speed so far.</param>
//void WindSpeed2::addGust_10_min(list<dataPoint>& targetList, float max, float min) {
//	float gust = 0;
//	// Gust must meet criteria.
//	if (
//		max >= GUST_THRESHOLD				// must exceed threshold, and
//		&& ((max - min) >= GUST_SPREAD)		// must exceed minimum by by GUST_SPREAD
//		) {
//		gust = max;							// New gust.
//	}
//	// Add gust to 10-min gust list.
//	_gust_last_10_min = gust;
//	addToList(targetList, dataPoint(_dataLastAdded.time, gust), SIZE_10_MIN_LIST);
//}

/// <summary>
/// Returns wind speed description in Beaufort 
/// wind strength scale.
/// </summary>
/// <param name="speed">Wind speed.</param>
/// <returns>Beaufort wind strength description.</returns>
String WindSpeed2::beaufortWind(float speed)
{
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
