// dataPoint.h

#ifndef _DATAPOINT_h
#define _DATAPOINT_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <list>
using std::list;

/// <summary>
/// A single data point of (time, value).
/// </summary>
struct dataPoint {
	unsigned long time;
	float value;

	/// <summary>
	/// A single data point of (time, value).
	/// </summary>
	/// <param name="time">Time (seconds since Jan 1 1970).</param>
	/// <param name="value">Data value.</param>
	dataPoint(unsigned long time = 0, float value = 0) :
		time(time), value(value)
	{	}
};

#endif
