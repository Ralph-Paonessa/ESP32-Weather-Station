// 
// 
// 

#include "WindDirection.h"

/// <summary>
/// Create with an angle offset to subtract from readings.
/// </summary>
/// <param name="offsetAngle">
/// Degrees by which reading exceeds true north.
/// </param>
WindDirection::WindDirection(float offsetAngle) {
	_offsetAngle = offsetAngle;
}

/// <summary>
/// Create the WindDirection object.
/// </summary>
void WindDirection::begin() {
	WindDirection::clear_10_min();
}

/// <summary>
/// Clears all direction values and averages.
/// </summary>
void WindDirection::clear_10_min() {
	_eSum = 0;
	_nSum = 0;
}

/*
		This solves the problem of updating the read time, even
		though we are ignoring the wind direction at low speeds.

		But what float value of degrees will be added to the list
		of dataPoint(time, value)?! We can't save an item to the
		10-min_today list (and longer) without a numerical value for angle!

		Could also get around this problem by
			- Adding all angle readings
			- WEIGHTING the avg angle by speed.
			- Saving the (possibly meaningless) direction at each 10-min_today
			- Not reporting a direction at times when speed < 1. (Easier
				to do with output strings).

		WEIGHTING BY SPEED will suppress the meaningless directions
		during the 10-min_today average period. If the speed is always zero
		(for all 10 min_today), it will still produce a meaningless direction
		number that will be logged. The REPORTING ROUTINE MUST IGNORE
		DIRECTIONS ASSOCIATED WITH NO SPEED.

		NOTE: If direction angle is logged as exactly zero, (which would
		be the case for a 10-min_today period with no wind)then web server
		returns it as "" for that time.
*/

/// <summary>
/// Adds wind direction reading for calculating 10-min_today 
/// average direction, weighted by speed.
/// </summary>
/// <param name="time">Reading time, sec.</param>
/// <param name="degrees">Wind direction (uncorrected), deg.</param>
/// <param name="speed">Speed at time of reading, mph.</param>
void WindDirection::addReading(long time, float degrees, float speed) {
	_dataPointLastAdded = dataPoint(time, degrees);
	///////////_timeLastAdded = time;
	// Only record direction for speeds greater than threshold.
	if (speed >= WIND_DIRECTION_SPEED_THRESHOLD) {
		degrees -= _offsetAngle;	// Adjust for any offset.
		// Converts angle into North and East components of
		// the direction vector and sums each component.
		// Weight these by the speed.
		_eSum += e_Component(degrees) * speed;
		_nSum += n_Component(degrees) * speed;
	}
}

/// <summary>
/// Returns East component of vector based on angle.
/// </summary>
/// <param name="degrees">Angle, degrees.</param>
/// <returns></returns>
double WindDirection::e_Component(float degrees) {
	return sin(degrees / DEGREES_PER_RADIAN);
}

/// <summary>
/// Returns North component of vector based on angle.
/// </summary>
/// <param name="degrees">Angle, degrees.</param>
/// <returns></returns>
double WindDirection::n_Component(float degrees) {
	return cos(degrees / DEGREES_PER_RADIAN);
}

/// <summary>
/// Returns angle (0-360 deg) of direction vector 
/// using North and East components from any of 
/// four quadrants.
/// </summary>
/// <param name="n">North component.</param>
/// <param name="e">East component.</param>
/// <returns></returns>
float WindDirection::angleFromComponents(double n, double e) {
	/*
		tan = Opposite/Adjacent
		tan = y/x -> normal xy coordinates, as used in atan2(y, x)

	Note: atan(y/x) does not distinguish quadrants from 0-360!
	atan2(y, x) does, but returns angles between 180-360 as
	negative angles between 0-180.

	Parameters for angle = atan2 are atan2(y, x) based on
	normal xy cartesian coordinates. The angle is defined
	as starting at the x-axis and increasing in a CCW
	direction towards the y-axis. Adjacent = y and Opposite = x.

	In the definition of direction (azimuth) angle, North = 0 and
	angle increases CW toward East = 90. For that angle, Adj = n
	and Opp = e. Therefore angle = atan2(n, e)  ~  arctangent(e, n)

		atan2(y, x) = atan2(Adj, Opp) = atan2(n, e)

	The azimuth coordinates are similar to polar coordinates but
	the direction of rotation is opposite. In that sense

		angle(azimuth) = 360 - angle(polar)
	*/
	float angle = atan2(n, e) * DEGREES_PER_RADIAN;
	if (angle < 0) {
		angle = 360 + angle;
	}
	return angle;
}

/// <summary>
/// Average wind direction from current accumulated readings, deg.
/// </summary>
/// <returns>Average wind direction.</returns>
float WindDirection::angleAvg_now() {
	return angleFromComponents(_eSum, _nSum);
}

/// <summary>
/// Returns latest cardinal direction using 
/// latest 10-min_today avg wind angle.
/// </summary>
/// <returns>Cardinal direction as string.</returns>
String WindDirection::directionCardinal() {
	return directionCardinal(_avg_10_min);
}

/// <summary>
/// Return cardinal direction for specified wind angle (deg).
/// </summary>
/// <param name="angle">Wind angle, deg.</param>
/// <returns>Wind cardinal direction.</returns>
String WindDirection::directionCardinal(float angle) {
	if (angle < 22)
		return "N";
	else if (angle < 67)
		return "NE";
	else if (angle < 112)
		return "E";
	else if (angle < 157)
		return "SE";
	else if (angle < 212)
		return "S";
	else if (angle < 247)
		return "SW";
	else if (angle < 292)
		return "W";
	else if (angle < 337)
		return "NW";
	else
		return "N";
}

/// <summary>
/// Returns list of cardinal directions from list of wind angles.
/// </summary>
/// <param name="angles">List containing wind angles.</param>
/// <returns>List of cardinal direction strings.</returns>
list<String> WindDirection::directions_cardinal(list<float>& angleList) {
	// Create list to hold cardinal direction strings.
	list<String> cardinals;
	//int i = 0;
	for (list<float>::iterator it = angleList.begin(); it != angleList.end(); ++it) {
		// Add the corresponding cardinal directions.
		cardinals.push_back(directionCardinal(*it));
	}
	return cardinals;
}

/// <summary>
/// Returns the vector-averaged angle for the specified 
/// number of elements of a list of data points, starting 
/// from the last element.
/// </summary>
/// <param name="targetList">List of data points with angle values.</param>
/// <param name="numElements">Number of elements from the end to average.</param>
/// <returns>Average angle, degrees.</returns>
float WindDirection::angleAvg_ofList(list<dataPoint>& targetList, int numElements) {
	double xSum = 0, ySum = 0;
	// Ensure we don't iterate past the first element.
	if (numElements > targetList.size()) {
		numElements = targetList.size();
	}
	// Iterate through the last (most recent) numElements elements.
	auto it = targetList.rbegin();	// Reverse iterator to last element.
	for (int i = 0; i < numElements; i++)
	{
		dataPoint dp = *it;
		xSum += e_Component(dp.value);
		ySum += n_Component(dp.value);
		it++;	// next element.
	}
	return angleFromComponents(xSum, ySum);	// count cancels out!
}