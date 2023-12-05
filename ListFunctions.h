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

	/// <summary>
	/// Adds dataPoint to list and limits list size. (If adding 
	/// creates too many elements, the first element is removed.)
	/// </summary>
	/// <param name="targetList">List of dataPoints to add to.</param>
	/// <param name="dp">dataPoint to add.</param>
	/// <param name="numElements">Maximum allowed elements in list.</param>
	void addToList(list<dataPoint>& targetList, dataPoint val, int numElements);

	/// <summary>
	/// Adds values to list and limits list size. (If adding 
	/// creates too many elements, the first element is removed.)
	/// </summary>
	/// <param name="targetList">List of values to add to.</param>
	/// <param name="val">Value to add.</param>
	/// <param name="numElements">Maximum allowed elements in list.</param>
	void addToList(list<float>& targetList, float val, int numElements);

	/// <summary>
	/// Returns the average of the last values of members 
	/// of a dataPoint list.
	/// </summary>
	/// <param name="targetList">
	/// The list of dataPoints to average.</param>
	/// <param name="numToAverage">
	/// The number of elements at the end of the list to average.
	/// </param>
	/// <returns>Average value.</returns>
	float listAverage(list<dataPoint>& targetList, int numElements);

	/// <summary>
	/// Returns the average of the last values of members 
	/// of a list of values.
	/// </summary>
	/// <param name="targetList">
	/// The list of values to average.</param>
	/// <param name="numToAverage">
	/// The number of elements at the end of the list to average.
	/// </param>
	/// <returns>Average value.</returns>
	float listAverage(list<float>& targetList, int numElements);

	/// <summary>
	/// Returns the largest value of a list of dataPoints 
	/// from the last numElements.
	/// </summary>
	/// <param name="targetList">List of dataPoint to check.</param>
	/// <param name="numElements">Number of elements to check, starting from end.</param>
	/// <returns>Largest value of a list</returns>
	float listMaximum(list<dataPoint>& targetList, int numElements);

	/// <summary>
	/// Converts a list of dataPoint to a string of "time, value" pairs 
	/// each delimited by "," and pairs delimited by "~".
	/// </summary>
	/// <param name="targetList">List to parse.</param>
	/// <returns>Delimited string of multiple (time, value) data points.</returns>
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
		list<dataPoint>& targetList,
		bool isConvertZeroToEmpty,
		unsigned int decimalPlaces);


	/// <summary>
	/// Converts two lists of dataPoints to strings of 
	/// comma-separated "time,value" pairs delimited by "~".
	/// Then combines both lists, delimited by "|".
	/// </summary>
	/// <param name="targetList_hi">First list of dataPoints.</param>
	/// <param name="targetList_lo">Second list of dataPoints.</param>
	/// <param name="isConvertZeroToEmpty">
	/// Set true to convert zero values to empty strings.
	/// </param>
	/// <param name="decimalPlaces">
	/// Decimal places to display.</param>
	/// <returns>Two String lists, respectively delimited by "|".</returns>
	String listToString_dataPoints(
		list<dataPoint>& targetList_hi,
		list<dataPoint>& targetList_lo,
		bool isConvertZeroToEmpty,
		unsigned int decimalPlaces);


	list<std::string> substrings_from_String(const String& str, const char delimiter);

	
	/// <summary>
	/// Returns a list of dataPoints retrieved from a delimited 
	/// string of comma-separated "time,value" pairs.
	/// </summary>
	/// <param name="delim">Delimited string of dataPoints.</param>
	/// <returns>
	/// List of "time,value" dataPoints retrieved from a delimited string.
	/// </returns>
	list<dataPoint> listFromString_dataPoints(String& delim);

	
	
	// dataPoint dataPointFromString(string& s);


	
	/*/// <summary>
	/// Converts two lists of dataPoints to strings of 
	/// comma-separated "time,value" pairs delimited by "~".
	/// Then combines both lists, delimited by "|".
	/// </summary>
	/// <param name="targetList_hi">First list of dataPoints.</param>
	/// <param name="targetList_lo">Second list of dataPoints.</param>
	/// <param name="isConvertZeroToEmpty">
	/// Set true to convert zero values to empty strings.
	/// </param>
	/// <param name="decimalPlaces">
	/// Decimal places to display.</param>
	/// <returns>Two String lists, respectively delimited by "|".</returns>
	String listFromString_dataPoints(
		list<dataPoint>& targetList_hi,
		list<dataPoint>& targetList_lo,
		bool isConvertZeroToEmpty,
		unsigned int decimalPlaces);*/


};

#endif
