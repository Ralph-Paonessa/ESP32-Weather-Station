// 
// 
// 

#include "ListFunctions.h"

#include <sstream>
#include <string>
using std::list;
using std::string;

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
void ListFunctions::addToList(list<float>& targetList, float val, int numElements) {
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
	for (int i = 1; i < numElements + 1; i++) {
		dataPoint dp = *it;
		if (dp.value > maxItem) {
			maxItem = dp.value;
		}
		it++;	// next item.
	}
	return maxItem;
}

/// <summary>
/// Converts a list of data points to a string of "time, value" 
/// pairs, each delimited by "," separate points delimited by 
/// "~". Such as "t1,v1~t2,v2~t3,v3".
/// </summary>
/// <param name="targetList">List of dataPoint.</param>
/// <returns>Delimited string of multiple (time, value) data points.</returns>
String ListFunctions::listToString_data(list<dataPoint>& targetList) {
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
String ListFunctions::listToString_data(
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
/// <returns>Two String lists, respectively delimited by "|".
/// </returns>
String ListFunctions::listToString_data(
	list<dataPoint>& targetList_hi,
	list<dataPoint>& targetList_lo,
	bool isConvertZeroToEmpty,
	unsigned int decimalPlaces)
{
	String s = "";
	if (!targetList_hi.size() == 0) {
		s += listToString_data(
			targetList_hi,
			isConvertZeroToEmpty,
			decimalPlaces);
	}
	else {
		s += "[-EMPTY HI-]";
	}
	s += "|";	// delimiter between lists
	if ((!targetList_lo.size() == 0)) {
		s += listToString_data(
			targetList_lo,
			isConvertZeroToEmpty,
			decimalPlaces);
	}
	else {
		s += "[-EMPTY LO-]";
	}
	return s;
}

///// <summary>
///// Splits a delimited string into a list of C++ std::string.
///// </summary>
///// <param name="str">String to split.</param>
///// <param name="delim">Delimiter character.</param>
///// <returns>List of substrings after splitting.</returns>
//list<std::string> ListFunctions::splitString(const String& str, const char delim) {
//	list<std::string> substrings;
//	std::istringstream ss(str.c_str());	// Convert input String to stream.
//	while (!ss.eof()) {
//		std::string sub;				// empty string to hold substrings
//		std::getline(ss, sub, delim);	// Read character stream before delimiter.
//		// Convert string to data point and add to list.
//		/*size_t i = sub.find_first_of(delim);
//		dataPoint dp = dataPoint(std::stoul(sub.substr(0, i)), std::stof(sub.substr(i + 1)));*/
//		substrings.push_back(sub);
//	}
//	return substrings;
//}

/// <summary>
/// Splits a delimited string into a list of Arduino String.
/// </summary>
/// <param name="str">String to split.</param>
/// <param name="delim">Delimiter char, such as ','.</param>
/// <returns>List of Strings after splitting.</returns>
list<String> ListFunctions::splitString(const String& str, const char delim) {
	list<String> substrings;
	std::istringstream ss(str.c_str());	// Convert input String to stream.
	while (!ss.eof()) {
		std::string sub;				// empty string to hold substrings
		std::getline(ss, sub, delim);	// Read character stream before delimiter.
		// Convert string to data point and add to list.
		substrings.push_back(String(sub.c_str()));
	}
	return substrings;
}

/// <summary>
/// Returns a list of dataPoints retrieved from a delimited 
/// string of comma-separated "time,value" pairs.
/// </summary>
/// <param name="str">Delimited string of dataPoints.</param>
/// <returns>
/// List of "time,value" dataPoints retrieved from a delimited string.
/// </returns>
list<dataPoint> ListFunctions::listData_fromString(String& str) {
	list<dataPoint> dPoints;		// List to hold data points.
	std::istringstream ss(str.c_str());
	while (!ss.eof()) {
		std::string sub;			// empty string to hold substrings
		std::getline(ss, sub, '~');	// Read next delimited field into sub.
		// Convert string to data point and add to list.
		size_t i = sub.find_first_of(",");
		float val = 0;
		std::string s = sub.substr(i + 1);
		// Replace empty values with zero.
		if (!s.empty()) {
			val = std::stof(s);
		}
		else {
			val = 0;
		}
		dataPoint dp = dataPoint(std::stoul(sub.substr(0, i)), val);
		dPoints.push_back(dp);
	}
	return dPoints;
}

void ListFunctions::listPrint(list<std::string> targetList) {
	Serial.println("List elements:");
	for (list<std::string>::iterator it = targetList.begin(); it != targetList.end(); ++it) {
		std::string s = *it;
		Serial.println(s.c_str());
	}
}

void ListFunctions::listPrint(list<String> targetList) {
	Serial.println("List elements:");
	for (list<String>::iterator it = targetList.begin(); it != targetList.end(); ++it) {
		String s = *it;
		Serial.println(s);
	}
}

void ListFunctions::listPrint(list<dataPoint> targetList) {
	Serial.println("List elements:");
	for (list<dataPoint>::iterator it = targetList.begin(); it != targetList.end(); ++it) {
		dataPoint dp = *it;
		Serial.println("(" + String(dp.time) + ", " + String(dp.value) + ")");
	}
}