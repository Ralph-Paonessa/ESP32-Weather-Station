// WindDirection.h

#ifndef _WINDDIRECTION_h
#define _WINDDIRECTION_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "SensorData.h"

/// <summary>
/// Object that reads and handles wind direction measurements.
/// </summary>
class WindDirection : public SensorData {	// Inherits from SensorData.

private:

	float _offsetAngle = 0;		// Degrees that reading exceeds true North.

	double _eSum = 0;	// Accumulated sum of x-component of direction vector.
	double _nSum = 0;	// Accumulated sum of y-component of direction vector.
	float _bearing = 0;	// Wind direction in degrees.

	float angleFromComponents(double n, double e);
	double e_Component(float degrees);
	double n_Component(float degrees);

	/// <summary>
	/// Return cardinal direction for specified wind angle (deg).
	/// </summary>
	/// <param name="angle">Wind angle, deg.</param>
	/// <returns>Wind cardinal direction.</returns>
	String directionCardinal(float angle);

public:

	// Constructor

	/// <summary>
	/// Initialize with an angle offset to subtract from readings.
	/// </summary>
	/// <param name="offsetAngle">
	/// Degrees by which reading exceeds true north.
	/// </param>
	WindDirection(float offsetAngle = 0);

	/// <summary>
	/// Initialize the WindDirection object.
	/// </summary>
	void begin();

	/// <summary>
	/// Clears all direction values and averages.
	/// </summary>
	void clearAverage();

	/// <summary>
	/// Adds wind direction reading for calculating 10-min 
	/// average direction, weighted by speed.
	/// </summary>
	/// <param name="time">Reading time, sec.</param>
	/// <param name="degrees">Wind direction (uncorrected), deg.</param>
	/// <param name="speed">Speed at time of reading, mph.</param>
	void addReading(long time, float degrees, float speed);	// OVERLOAD

	// <summary>
	/// Average wind direction from current accumulated readings, deg.
	/// </summary>
	/// <returns>Average wind direction.</returns>
	float angleAvg_now();

	/// <summary>
	/// Returns latest cardinal direction using 
	/// latest 10-min avg wind angle.
	/// </summary>
	/// <returns>Cardinal direction as string.</returns>
	String directionCardinal();

	/// <summary>
	/// Returns list of cardinal directions from list of wind angles.
	/// </summary>
	/// <param name="angles">List containing wind angles.</param>
	/// <returns>List of cardinal direction strings.</returns>
	list<String> directions_cardinal(list<float>& angleList);

	/// <summary>
	/// Returns the vector-averaged angle for the specified 
	/// number of elements of a list of data points, starting 
	/// from the last element.
	/// </summary>
	/// <param name="targetList">List of data points with angle values.</param>
	/// <param name="numElements">Number of elements from the end to average.</param>
	/// <returns>Average angle, degrees.</returns>
	float angleAvg_ofList(list<dataPoint>& targetList, int numElements);
};

#endif
