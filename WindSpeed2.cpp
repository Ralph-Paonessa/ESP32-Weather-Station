// 
// 
// 

#include "WindSpeed2.h"

// Constructor.

/// <summary> Creates WindSpeed instance that exposes 
/// methods to read and process wind speed data.
/// </summary>
/// <param name="calibrationFactor">
/// Calibration factor for anemometer.</param>
/// <param name="isUseSmoothing">
/// Set true to smooth data (default = false).</param>
/// <param name="numValuesForAvg">	
/// Number of values in moving average (default = 5).</param>
/// <param name="outlierDelta">
/// Factor applied to moving avg for outlier comparison (default = 1.75).</param>
WindSpeed::WindSpeed(
	float calibrationFactor,
	bool isUseSmoothing,
	unsigned int numValuesForAvg,
	float outlierDelta
)
{
	_calibrationFactor = calibrationFactor;
	_isUseSmoothing = isUseSmoothing;
	_avgMoving_Num = numValuesForAvg;
	_outlierDelta = outlierDelta;
}

/// <summary>
/// Returns wind speed from anemometer rotations.
/// </summary>
/// <param name="rotations"> Number of rotations.</param>
/// <param name="period">Time period of rotations, sec.</param>
/// <returns>Wind speed, mph</returns>
float WindSpeed::speedInstant(int rotations, float period)
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
/// gust criteria. Otherwise, the returned value will be zero if 
/// the speed doesn't satisfy gust criteria.
/// </summary>
/// <param name="speed">(time, value) point to evaluate for gust.</param>
/// <param name="avgSpeed">Current average wind speed.</param>
/// <returns>Gust as (time, value) data point.</returns>
dataPoint WindSpeed::gust(dataPoint speed, float avgSpeed)
{
	if (
		// If Gust exceeds threshold
		speed.value >= GUST_THRESHOLD
		&&
		// If gust exceeds moving avg by GUST_SPREAD
		((speed.value - _avgMoving) >= GUST_SPREAD)	// Inherit moving avg from SensorData.
		)
	{		// Report this as a gust.
		return speed;
	}
	else {
		// Not a gust, so return value of zero.
		return dataPoint(speed.time, 0);
	}
}

/// <summary>
/// Returns wind speed description in Beaufort 
/// wind strength scale.
/// </summary>
/// <param name="speed">Wind speed.</param>
/// <returns>Beaufort wind strength description.</returns>
String WindSpeed::beaufortWind(float speed)
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
