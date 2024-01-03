// WindSpeed2.h

#ifndef _WINDSPEED2_h
#define _WINDSPEED2_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#include <list>
using std::list;

#include "ListFunctions.h"
#include "App_settings.h"

using namespace ListFunctions;
using namespace App_Settings;

#include "SensorData.h"

/// <summary>
/// Exposes methods to measure and record wind speeds.
/// </summary>
class WindSpeed : public SensorData {	// Inherits SensorData.

private:

	float _calibrationFactor;			// Anemometer calibration factor to convert rotations to speed.

	// For identification of gusts.
	const float GUST_THRESHOLD = 18.41;	// WindSpeed must exceed this to be a gust.
	const float GUST_SPREAD = 10.36;	// WindSpeed must exceed low by this amount to be a gust.

public:

	// Constructor	

	/// <summary> Initializes WindSpeed instance that exposes 
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
	WindSpeed(
		float calibrationFactor,
		bool isUseMovingAvg = false,
		unsigned int numSmoothPoints = 5,
		float outlierDelta = 1.75);		// Overload of SensorData.

	/// <summary>
	/// Returns wind speed from anemometer rotations.
	/// </summary>
	/// <param name="rotations"> Number of rotations.</param>
	/// <param name="period">Time period of rotations, sec.</param>
	/// <returns>Wind speed, mph</returns>
	float speedInstant(int rotations, float period);
		
	/// <summary>
	/// Checks for and returns a gust datPoint if the speed satisfies 
	/// gust criteria. Otherwise, the returned value will be zero if 
	/// the speed doesn't satisfy gust criteria.
	/// </summary>
	/// <param name="speed">(time, value) point to evaluate for gust.</param>
	/// <param name="avgSpeed">Current average wind speed.</param>
	/// <returns>Gust as (time, value) data point.</returns>
	dataPoint gust(dataPoint speed, float avgSpeed);

	/// <summary>
	/// Returns wind speed description in Beaufort 
	/// wind strength scale.
	/// </summary>
	/// <param name="speed">Beaufort wind strength.</param>
	/// <returns></returns>
	String beaufortWind(float speed);
};

#endif
