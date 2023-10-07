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

	float listAverage(list<dataPoint>& targetList, int numElements);

	float listMaximum(list<dataPoint>& targetList, int numElements);

	String listToString_dataPoints(list<dataPoint>& targetList);

	String listToString_dataPoints(
		list<dataPoint> targetList,
		bool isConvertZeroToEmpty,
		int decimalPlaces);
};

#endif
