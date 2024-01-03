// 
// 
// 

#include "ListFunctions.h"

using std::list;

/////   LIST MANIPULATIONS   /////

/// <summary>
/// Adds dataPoint to list and limits list size. (If adding 
/// creates too many elements, the first element is removed.)
/// </summary>
/// <param name="targetList">List of dataPoints to add to.</param>
/// <param name="dp">dataPoint to add.</param>
/// <param name="numElements">Maximum allowed elements in list.</param>
void ListFunctions::addToList(list<dataPoint>& targetList, dataPoint dp, int numElements) {
	targetList.push_back(dp);		// Add to list (raw).
	if (targetList.size() > numElements) {
		targetList.pop_front();		// If too many, remove the first.
	}
}

/// <summary>
/// Adds values to list and limits list size. (If adding 
/// creates too many elements, the first element is removed.)
/// </summary>
/// <param name="targetList">List of values to add to.</param>
/// <param name="val">Value to add.</param>
/// <param name="numElements">Maximum allowed elements in list.</param>
void ListFunctions::addToList(list<float>& targetList, float val, int numElements)
{
	targetList.push_back(val);		// Add value to list.
	if (targetList.size() > numElements) {
		targetList.pop_front();		// If too many, remove the first.
	}
}

/// <summary>
/// Returns the average of the end values of members 
/// of a dataPoint list.
/// </summary>
/// <param name="targetList">
/// The list of dataPoint to average.</param>
/// <param name="numToAverage">
/// The number of elements at the end of the list to average.
/// </param>
/// <returns>Average value.</returns>
float ListFunctions::listAverage(list<dataPoint>& targetList, int numToAverage) {
	// Ensure we don't iterate past the first element.
	if (numToAverage > targetList.size()) {
		numToAverage = targetList.size();
	}
	// Iterate through the last (most recent) elements.
	auto it = targetList.rbegin();	// Reverse iterator to last element.
	float total = 0;
	for (int i = 0; i < numToAverage; i++)
	{
		dataPoint dp = *it;
		total += dp.value;
		it++;
	}
	return total / numToAverage;
}

/// <summary>
/// Returns the average of the end values of members 
/// of a dataPoint list.
/// </summary>
/// <param name="targetList">
/// The list of dataPoint to average.</param>
/// <param name="numToAverage">
/// The number of elements at the end of the list to average.
/// </param>
/// <returns>Average value.</returns>
float ListFunctions::listAverage(list<float>& targetList, int numToAverage) {
	// Ensure we don't iterate past the first element.
	if (numToAverage > targetList.size()) {
		numToAverage = targetList.size();
	}
	// Iterate through the last (most recent) elements.
	auto it = targetList.rbegin();	// Reverse iterator to last element.
	float total = 0;
	for (int i = 0; i < numToAverage; i++)
	{
		total += *it;
		it++;
	}
	return total / numToAverage;
}




/// <summary>
/// Returns the largest value of a list of dataPoint from the last numElements.
/// </summary>
/// <param name="targetList">List of dataPoint to check.</param>
/// <param name="numElements">Number of elements to check, starting from end.</param>
/// <returns>Largest value of a list</returns>
float ListFunctions::listMaximum(list<dataPoint>& targetList, int numElements) {
	// Ensure we don't iterate past the first element.
	if (numElements > targetList.size()) {
		numElements = targetList.size();
	}
	// Iterate through the last (most recent) elements.
	auto it = targetList.rbegin();	// Reverse iterator to last element.
	// Start with low value we should never see.
	float maxItem = -999999999;
	for (int i = 1; i < numElements + 1; i++)
	{
		dataPoint dp = *it;
		if (dp.value > maxItem) {
			maxItem = dp.value;
		}
		it++;	// next item.
	}
	return maxItem;
}

/// <summary>
/// Converts a list of dataPoint to a string of "time, value" pairs 
/// each delimited by "," and pairs delimited by "~".
/// </summary>
/// <param name="targetList">List to parse.</param>
/// <returns>Delimited string of multiple (time, value) data points.</returns>
String ListFunctions::listToString_dataPoints(list<dataPoint>& targetList)
{
	String s = "";
	if (targetList.size() == 0) {
		return s + "[-EMPTY-]";
	}
	dataPoint dp{};
	for (list<dataPoint>::iterator it = targetList.begin(); it != targetList.end(); ++it) {
		// Output each dataPoint as CSV separated by "~".
		s += String(it->time) + ",";
		s += String(it->value) + "~";
	}
	return s.substring(0, s.length() - 1);	// remove final delimiter
}

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
String ListFunctions::listToString_dataPoints(
	list<dataPoint>& targetList,
	bool isConvertZeroToEmpty,
	unsigned int decimalPlaces)
{
	String s = "";
	if (targetList.size() == 0) {
		return s + "[-EMPTY-]";
	}
	for (list<dataPoint>::iterator it = targetList.begin(); it != targetList.end(); ++it) {
		dataPoint dp = *it;
		s += String(dp.time) + ",";
		if (isConvertZeroToEmpty && dp.value == 0)
		{
			s += "~";	// Leave value empty in string.
		}
		else {
			s += String(dp.value, decimalPlaces) + "~";
		}
	}
	return s.substring(0, s.length() - 1);	// remove final delimiter
}

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
/// <returns>Two lists delimited by "|".</returns>
String ListFunctions::listToString_dataPoints(
	list<dataPoint>& targetList_hi,
	list<dataPoint>& targetList_lo,
	bool isConvertZeroToEmpty,
	unsigned int decimalPlaces)
{
	String s = "";
	if (!targetList_hi.size() == 0)
	{
		s += listToString_dataPoints(targetList_hi, isConvertZeroToEmpty, decimalPlaces);
	}
	else
	{
		s += "[-EMPTY HI-]";
	}
	s += "|";	// delimiter between lists
	if ((!targetList_lo.size() == 0))
	{
		s += listToString_dataPoints(targetList_lo, isConvertZeroToEmpty, decimalPlaces);
	}
	else
	{
		s += "[-EMPTY LO-]";
	}
	return s;
}