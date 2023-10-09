// ListFunctions.h

#ifndef _LISTFUNCTIONS_h
#define _LISTFUNCTIONS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "dataPoint.h"

#include <list>
using std::list;

/// <summary>
/// Exposes methods that operate on lists.
/// </summary>
namespace ListFunctions {

	void addToList(list<dataPoint>& targetList, dataPoint val, int numElements);

	/// <summary>
	/// Returns the average of the last values of members 
	/// of a dataPoint list.
	/// </summary>
	/// <param name="targetList">
	/// The list of dataPoint to average.</param>
	/// <param name="numToAverage">
	/// The number of elements at the end of the list to average.
	/// </param>
	/// <returns>Average.</returns>
	float listAverage(list<dataPoint>& targetList, int numElements);

	float listMaximum(list<dataPoint>& targetList, int numElements);

	String listToString_dataPoints(list<dataPoint>& targetList);

	/// <summary>
	/// Converts a list of dataPoints to a string of 
	/// comma-separated "time,value" pairs delimited by "~".
	/// </summary>
	/// <param name="targetList">List of dataPoints.</param>
	/// <param name="isConvertZeroToEmpty">
	/// Set true to convert zero value to empty string.</param>
	/// <param name="decimalPlaces">Decimal places to display.</param>
	/// <returns>
	/// Comma-separated "time,value" pairs delimited by "~"</returns>
	String listToString_dataPoints(
		list<dataPoint> targetList,
		bool isConvertZeroToEmpty,
		unsigned int decimalPlaces);
};

#endif
