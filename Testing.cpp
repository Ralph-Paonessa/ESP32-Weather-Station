// 
// 
// 

#include "Testing.h"
#include "ListFunctions.h"



/// <summary>
/// Test code to insert in setup for debugging.
/// </summary>
/// <param name="runTime_sec">Number of seconds to run.</param>
void Testing::testCodeForSetup1(unsigned long runTime_sec, bool isInfiniteLoop) {
	Serial.println(LINE_SEPARATOR);
	Serial.print("TEST in setup to run for "); Serial.print(runTime_sec); Serial.println(" sec/n");
	unsigned long timeStart = millis();
	/********************************/
	/* INSERT DEFINITIONS HERE.     */


	/********************************/
	/*while (millis() < timeStart + runTime_sec * 1000)
	{*/
	/********************************/
	/* INSERT TEST CODE HERE.       */

	/*Serial.println("d_Temp_F.data_10_min_string():");
	Serial.println(d_Temp_F.data_10_min_string()); Serial.println();

	list<dataPoint> dpList = d_Temp_F.recovered_data_10_min();
	String s = listToString_data(dpList);
	Serial.println("String from list from flash:");
	Serial.println(s);*/

	/*
	list<float> testList;
	float val = 10;
	for (size_t i = 0; i < 5; i++)
	{
		val += i;
		addToList(testList, val, 5);
	}
	float avg = listAverage(testList, 5);
	Serial.printf("list avg = %f", avg);
	*/


	/********************************/
/*}*/
	Serial.println("TEST COMPLETE");
	Serial.println(LINE_SEPARATOR);
	if (isInfiniteLoop) {
		infiniteLoop();
	}
}


/// <summary>
/// Test code to insert in setup for debugging.
/// </summary>
void Testing::testCodeForSetup2(bool isInfiniteLoop) {
	Serial.println(LINE_SEPARATOR);
	Serial.println("TEST START in setup()");
	unsigned long timeStart = millis();

	String s2 = "1,64~2,~3,63~4,~5,61";
	Serial.println("String to convert to list:");
	Serial.println(s2);
	list<dataPoint> dpList2 = listData_fromString(s2);
	Serial.println("String derived from list of data points:");
	Serial.println(listToString_data(dpList2));

	Serial.println("TEST COMPLETE");
	Serial.println(LINE_SEPARATOR);
	if (isInfiniteLoop) {
		infiniteLoop();
	}
}

/// <summary>
/// Test code for ListFunctions::splitString.
/// </summary>
void Testing::testCodeForSetup3(bool isInfiniteLoop) {
	Serial.println(LINE_SEPARATOR);
	Serial.println("TEST START in setup()");
	unsigned long timeStart = millis();

	String s = "1,64~2,~3,63~4,~5,61";

	Serial.println("String to convert to list:");
	Serial.println(s);
	Serial.println("splitString(s, '~')");
	// Convert String to list.
	list<String> sList = splitString(s, '~');
	// Print the list elements.
	listPrint(sList);

	Serial.println("TEST COMPLETE");
	Serial.println(LINE_SEPARATOR);
	if (isInfiniteLoop) {
		infiniteLoop();
	}
}

/// <summary>
/// Test code for ListFunctions::splitString.
/// </summary>
void Testing::testCodeForSetup4(bool isInfiniteLoop) {
	//#if defined(VM_DEBUG)
	Serial.println(LINE_SEPARATOR);
	Serial.println("TEST START in setup()");
	unsigned long timeStart = millis();

	char delimiter = '|';
	Serial.print("Delimiter = "); Serial.println(delimiter);

	String s0 = "";
	String s1 = "1,64~2,~3,63~4,~5,61";
	String s2 = "1,64~2,~3,63~4,~5,61|10,64~20,~30,63~40,~50,61";

	String target[] = { s0, s1, s2 };

	for (int i = 0; i < 3; i++)	{
		Serial.println("String to split into a list:");
		Serial.println(target[i]);
		// Convert String to list.
		Serial.println("splitString(target[i], delimiter)");
		list<String> sList = splitString(target[i], delimiter);
		// Print the list elements.
		listPrint(sList);
		Serial.println(LINE_SEPARATOR);
	}

	Serial.println("TEST COMPLETE");
	Serial.println(LINE_SEPARATOR);
	if (isInfiniteLoop) {
		infiniteLoop();
	}
}

/// <summary>
/// Test code for ListFunctions::splitString.
/// </summary>
void Testing::testCodeForSetup5(bool isInfiniteLoop) {
#if defined(VM_DEBUG)
	Serial.println(LINE_SEPARATOR);
	Serial.println("TEST START in setup()");
	unsigned long timeStart = millis();

	std::string s = "";
	if (s.empty()) {
		Serial.println("s is Empty.");
	}
	else {
		Serial.println("s is NOT Empty.");
	}
	float val = std::stof(s);
	Serial.print("val = "); Serial.println(val);

	Serial.println("TEST COMPLETE");
	Serial.println(LINE_SEPARATOR);
	if (isInfiniteLoop) {
		infiniteLoop();
	}
}

void Testing::infiniteLoop() {
	Serial.println("Halting with infinite loop.");
	while (true) {}	// infinite loop to halt
#endif
}

float Testing::testDummyReading(float seed, float increment1, float increment2, int periods, unsigned long count)
{
	if (count == 1)
	{
		return seed;
	}
	if (count < periods + 1)
	{
		return _dummyVal += increment1 / periods;
	}
	if (count < 2 * periods + 1)
	{
		return _dummyVal += (increment1 + increment2) / periods;
	}
	if (count < 3 * periods + 1)
	{
		return _dummyVal += (increment1 + 2 * increment2) / periods;
	}
	if (count < 4 * periods + 1)
	{
		return _dummyVal += (increment1 + 3 * increment2) / periods;
	}
	if (count < 5 * periods + 1)
	{
		return _dummyVal += (increment1 + 4 * increment2) / periods;
	}
	return _dummyVal;
}

/// <summary>
/// Returns gradually increasing dummy rotation count over a number
/// of periods, with one spiked value at specified period.
/// </summary>
/// <param name="first">First value to return</param>
/// <param name="last">Last value to return</param>
/// <param name="spike">Amount to increase value at spike period</param>
/// <param name="spikePeriod">Period to spike value.</param>
/// <param name="periods">Total number of periods before reset.</param>
/// <param name="count"></param>
/// <returns></returns>
int Testing::dummyRotations(float first, float last, float spike, int spikePeriod, int periods)
{
	_countPeriods++;
	float incr = (last - first) / periods;		// increase by this amount
	float val = first + incr * _countPeriods;

	if (_countPeriods == spikePeriod)
	{
		val += spike;
	}
	if (_countPeriods == spikePeriod + 5)
	{
		val += 1.5 * spike;
	}
	if (_countPeriods == periods)
	{
		_countPeriods = 0;
	}
	return val;
}

// Returns simulated data as CSV string.
String Testing::readData()
{
	return "12.34,10,12.95,13,16,8.7234";
}

// Tests WindSpeed2.h wind directionCardinal handling.
void Testing::windDirection(float angleStart, float angleIncrement, int cycles, float VANE_OFFSET) {
	Serial.println("WindSpeed2.h windDirection test. Add deg readings and average.");
	WindDirection windDirect(VANE_OFFSET);
	windDirect.begin();
	for (int i = 0; i < cycles; i++)
	{
		///////////windDirect.addReading(now(), angleStart);   now() not defined in this scope!!!!
		////////////Serial.print("Readings = "); Serial.print(windDirect.countReadings(), 0);
		Serial.print(" - addReading("); Serial.print(angleStart);
		Serial.print(") ... \tangleAvg() = "); Serial.println(windDirect.angleAvg_now());
		angleStart += angleIncrement;
	}

	windDirect.clear_10_min();
	Serial.println("\nAFTER ALL VALUES CLEARED ...");
	///////////////Serial.print(" Readings = "); Serial.println(windDirect.countReadings(), 0);
	Serial.print("angleAvg_now() = "); Serial.println(windDirect.angleAvg_now());
}

//void Testing::test2() {
//	Serial.println("Test adding rotations counts from various optional lists.");
//	// Create lists with 48 values of dummy rot counts.
//	list<float> rotsList_all_10({ 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10 });
//	list<float> rotsList_varied({ 10,10,10,10,10,10,10,10,10,10,10,10,10,5,30,99,10,10,10,10,10,10,10,10,10,10,5,30,99,10,10,10,10,10,10,10,10,10,10,10,10,10,10,200,10,10,10,10 });
//	list<float> rotsList_with_zeros({ 10,0,0,10,10,0,10,10,10,0,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,0,0,0,10,0,0,0 });
//	list<float> rotsList_all_zeros({ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 });
//	list<float> rotsList_one_large({ 10,10,9990,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10 });
//	list<float> rotsList_all_20({ 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 });
//	/*Serial.print("List size rotsList_all_10 = "); Serial.println(rotsList_all_10.size());
//	Serial.print("List size rotsList_one_1000 = "); Serial.println(rotsList_one_1000.size());
//	Serial.print("List size rotsList_all_20 = "); Serial.println(rotsList_all_20.size());
//	Serial.println(listToString(rotsList_all_10, "rotsList_all_10");
//	Serial.println(listToString(rotsList_one_1000, "rotsList_one_1000");
//	Serial.println(listToString(rotsList_all_20, "rotsList_all_20");*/
//
//	float calFactor = 2.5;		// It's really 2.25!!!!  XXX  SHOULD THIS BE HERE OR IN CALLING PROGRAM?!?!?!
//	// Create a wind speed object.
//	WindSpeed windSpeed(calFactor);
//
//	float elapsed = 0;
//	int minutes1 = 58;
//	int minutes2 = 2;
//	int numRepeats = 1;
//
//	// Outer loop.
//	for (int repeat = 1; repeat <= numRepeats; repeat++)
//	{
//		// Loop 1 - 
//		for (int min_2 = 1; min_2 <= minutes1 / 2; min_2++)
//		{
//			// Every iteration adds 2 minutes.
//			// Dummy rotation counts (from a list).
//			elapsed += addDummyRotations(rotsList_all_zeros, windSpeed, BASE_PERIOD_SEC);
//			//elapsed += addDummyRotations(rotsList_one_large, wind, RAW_PERIOD);
//			//elapsed += addDummyRotations(rotsList_all_20, wind, RAW_PERIOD);
//			if (true) {
//				Serial.print("\nElapsed time = "); Serial.print(elapsed / 60.); Serial.println(" min");
//
//				Serial.print("Latest speedInstant 2-min = "); Serial.println(windSpeed.speed_last_2_min().value);
//				Serial.print("Latest speedInstant 10-min = "); Serial.println(windSpeed.speed_last_10_min().value);
//				Serial.print("Latest speedInstant 60-min = "); Serial.println(windSpeed.avg_60_min().value);
//				Serial.print("Latest gust 10-min = "); Serial.println(windSpeed.gust_last_10_min().value);
//				Serial.print("Latest gust 60-min = "); Serial.println(windSpeed.gust_last_60_min().value);
//				//Serial.println(listToString(wind.speeds_instant(), "raw");
//				Serial.println(listToString_data(windSpeed.speeds_2_min(), "2-min"));
//				Serial.println(listToString_data(windSpeed.data_10_min(), "10-min"));
//				Serial.println(listToString_data(windSpeed.gusts_10_min(), "Gusts 10-min"));
//				Serial.println(listToString_data(windSpeed.data_60_min(), "60-min"));
//				Serial.println(listToString_data(windSpeed.gusts_60_min(), "Gusts 60-min"));
//			}
//		}
//		// Loop 2 - 
//		for (int min_2 = 1; min_2 <= minutes2 / 2; min_2++)
//		{
//			// Every iteration adds 2 minutes.
//			// Dummy rotation counts (from a list).
//			//elapsed += addDummyRotations(rotsList_all_10, wind, BASE_PERIOD_SEC);
//			elapsed += addDummyRotations(rotsList_one_large, windSpeed, BASE_PERIOD_SEC);
//			//elapsed += addDummyRotations(rotsList_all_20, wind, BASE_PERIOD_SEC);
//			if (true) {
//				Serial.print("LOOP 2: "); Serial.print("Elapsed time = "); Serial.print(elapsed / 60.); Serial.println(" min");
//
//				Serial.print("Latest speedInstant 2-min = "); Serial.println(windSpeed.speed_last_2_min().value);
//				Serial.print("Latest speedInstant 10-min = "); Serial.println(windSpeed.speed_last_10_min().value);
//				Serial.print("Latest speedInstant 60-min = "); Serial.println(windSpeed.avg_60_min().value);
//				Serial.print("Latest gust 10-min = "); Serial.println(windSpeed.gust_last_10_min().value);
//				Serial.print("Latest gust 60-min = "); Serial.println(windSpeed.gust_last_60_min().value);
//
//				//Serial.println(listToString(wind.speeds_instant(), "raw");
//				Serial.println(listToString_data(windSpeed.speeds_2_min(), "2-min"));
//				Serial.println(listToString_data(windSpeed.data_10_min(), "10-min"));
//				Serial.println(listToString_data(windSpeed.gusts_10_min(), "Gusts 10-min"));
//				Serial.println(listToString_data(windSpeed.data_60_min(), "60-min"));
//				Serial.println(listToString_data(windSpeed.gusts_60_min(), "Gusts 60-min"));
//			}
//		}
//	}
//}

//void Testing::testList() {
//	Serial.println("Test list.");
//	float calFactor = 2.5;		// It's really 2.25!!!!  XXX
//	// Create a wind speed object.
//	WindSpeed windSpeed(calFactor);
//
//	list<float> test({ 1,2,3,4,5 });
//	/*test.push_back(100);
//	test.push_back(90);
//	test.push_back(80);
//	test.push_back(70);
//	test.push_back(200);
//	test.push_back(60);
//	test.push_back(50);*/
//	Serial.println(listToString(test, "testList"));
//	Serial.print("Max of last 3: "); Serial.println(listMaximum(test, 3));
//	//Serial.println(listToString(test, "testList"));
//	Serial.print("Max of all: "); Serial.println(listMaximum(test));
//	//Serial.println(listToString(test, "testList"));
//
//	Serial.print("Avg of all: "); Serial.println(listAverage(test));
//	//Serial.println(listToString(test, "testList"));
//	Serial.print("Avg of last 3: "); Serial.println(listAverage(test, 3));
//	Serial.println(listToString(test, "testList"));
//}


/// <summary>
/// Tests for list of dataPoint struct.
/// </summary>
/// <param name="time"></param>
void Testing::testStructList(long time)
{
	SensorData d_test;
	d_test.addLabels("Dummy test data", "test", "units");
	d_test.addDummy_data_10_min(30, .5, 15, time);

	Serial.println(d_test.label());

	Serial.println("10-min");
	Serial.println(d_test.data_10_min_string());
	Serial.println();

	Serial.println("10-min 1 decimal place.");
	Serial.println(d_test.data_10_min_string());
}

// Add a list of rotations to a WindSpeed object, and return elapsed seconds.
float Testing::addDummyRotations(list<float> srcList, WindSpeed& speedObj, float rawPeriod) {
	// Dummy rotation counts (from a list).
	float elapsed = 0;
	for (list<float>::iterator it = srcList.begin(); it != srcList.end(); ++it) {

		speedObj.addReading(dataPoint(999999, *it));
		elapsed += rawPeriod;
	}
	return elapsed;
}

//void Testing::test() {
//	Serial.println("Starting test ...");
//	float calFactor = 2.5;		// It's really 2.25!!!!  XXX
//
//	// Create a wind speed object.
//	WindSpeed windSpeed(calFactor);
//
//	float timeElapsed = 0.;		// keep track of elapsed time (simulated).
//	int countRawPeriods = 0;
//
//	int speedChangeCount = 0;
//	int speedChangeCountReset = 24;	// revert speed change after this many cycles
//	int speedChangeCrossover = 12;		// when to change speed. MUST BE LOWER THAN speedChangeCountReset!!!
//	int rots = 10;
//	int gustCount = 48 * 10;		// 2.5 sec x 480 = 1200 sec = 20 min --> every 20 min, add large speed.
//
//	// Read speed each cycle in this loop. Limit to numCycles.
//	int numCycles = 480;	// limit num of cycles
//	for (long cycle = 1; cycle <= +numCycles; cycle++)
//	{
//		timeElapsed += 2.5;		// simulated time, sec
//		countRawPeriods++;
//
//		// Periodicallly change speed (rotations) for some of the cycles.
//		// 1 cycle = 2.5 sec.
//		// 48 cycles = 2 min
//		// 240 cycles = 10 min
//		speedChangeCount++;
//		if (speedChangeCount < speedChangeCrossover) {
//			rots = 10;		// rotations at first.
//		}
//		else {
//			rots = 10;		// rotations changed.
//		}
//		if (speedChangeCount >= speedChangeCountReset) {
//			speedChangeCount = 0;	// Reset speed change.
//		}
//
//		// Do the actual reading!!!
//		windSpeed.addReading(rots, 999999);
//
//
//		Serial.print("\n---> CYCLE #"); Serial.print(cycle); Serial.print(". time = "); Serial.print(timeElapsed / 60.); Serial.println(" min");
//
//		Serial.println(listToString_data(windSpeed.speeds_instant(), "raw"));
//		Serial.println(listToString_data(windSpeed.speeds_2_min(), "2-min"));
//		Serial.println(listToString_data(windSpeed.data_10_min(), "10-min"));
//		Serial.println(listToString_data(windSpeed.data_60_min(), "60-min"));
//
//		Serial.print("10-min list size = "); Serial.println(windSpeed.data_10_min().size());
//
//		Serial.print("GUSTS 10-min count = "); Serial.println(windSpeed.gusts_10_min().size());
//		//Serial.println(listToString(wind.gusts_10_min());
//
//		Serial.print("60-min count = "); Serial.println(windSpeed.data_60_min().size());
//		//Serial.println(listToString(wind.data_60_min());
//
//		//Serial.print("GUSTS 60-min count = ")); Serial.println(wind.gusts_60_min().size());
//		//Serial.println(listToString(wind.gusts_60_min());
//
//		Serial.println(LINE_SEPARATOR);
//	}
//}

//void Testing::testTimeMonPassByReference(TimeMonitor& timeMon)
//{
	//_timeMonLocal = timeMon;
	//Serial.printf("_timeMonLocal base periods = %i\n", _timeMonLocal.countBasePeriods());
	//Serial.printf("Referenced TimeMonitor base periods = %i\n\n", timeMon.countBasePeriods());

	//_timeMonLocal.update(12345);
	//Serial.printf("_timeMonLocal base periods after update = %i\n", _timeMonLocal.countBasePeriods());
	//Serial.printf("Referenced TimeMonitor base periods = %i\n", timeMon.countBasePeriods());
//}

//void Testing::timeMonitorTest(unsigned long countBasePeriods, float basePeriodSeconds)
//{
//	TimeMonitor timeMon;	// Time monitor object.
//
//	timeMon.update(countBasePeriods);
//
//	float basePeriod = 2.5;
//	float elapsedTime = 0;
//	for (int i = 1; i < 4000; i++)
//	{
//		//countBasePeriods++;
//		elapsedTime += basePeriod;
//		Serial.print(i); Serial.print(" - ");
//
//		Serial.print("countBasePeriods = "); Serial.print(timeMon.countBasePeriods());
//		Serial.print(" = "); Serial.print(elapsedTime); Serial.print(" sec");
//
//		if (timeMon.is_2_min())
//		{
//			Serial.print(" - is 2 min **********");
//		}
//		if (timeMon.is_10_min())
//		{
//			Serial.print(" - is 10 min **********");
//		}
//		if (timeMon.is_60_min())
//		{
//			Serial.print(" - is 60 min **********");
//		}
//		Serial.println();
//	}
//}


////////// Print Raw Data (for testing)  /////////

//void Testing::Print_DS18B20_T(float tempC, float tempF) {
//	// Temperature.
//	Serial.print(F("DS18B20 T : \t"));
//	Serial.print(tempC, 1);
//	Serial.print(F("ºC "));
//	Serial.print(tempF, 1);
//	Serial.println(F("ºF"));
//}
//
//void Testing::Print_MLX90614_IR() {
//	Serial.print(F("MLX90614 IR : \t"));
//	if (mlx.readAmbientTempC() < 1000.) {
//		Serial.print(mlx.readObjectTempC());
//		Serial.print(F("ºC (object), "));
//		Serial.print(mlx.readAmbientTempC());
//		Serial.println(F("ºC (ambient)"));
//	}
//	else {
//		// Sensor was not found.
//		// (Missing sensor gives T > 1000)
//		Serial.println(F("* not found *"));
//	}
//}
//
//void Testing::Print_BME280_P_RH() {
//	Serial.print(F("BME280 : \t\t"));
//	if (sensor_PRH.create(0x77)) {
//		Serial.print(sensor_PRH.readPressure() / 100, 0);
//		Serial.print(F(" mb P\t")); // Pressure in mb
//		Serial.print(sensor_PRH.readHumidity(), 0);
//		Serial.println(F(" % RH"));
//	}
//	else {
//		// Sensor was not found.
//		Serial.println(F("* not found *"));
//	}
//}
//
//void Testing::Print_VEML6075_UV() {
//	Serial.print(F("VEML6075 : \t"));
//	if (uv.create()) {
//		Serial.print(String(uv.uva()));
//		Serial.print(F(" UV - A\t"));
//		Serial.print(String(uv.uvb()));
//		Serial.println(F(" UV - B"));
//	}
//	else {
//		Serial.println(F("* not found *"));
//	}
//}
//
//void Testing::PrintWindDirection() {
//	Serial.print(F("Wind directionCardinal : \t"));
//	Serial.print(F("Raw = ")); Serial.print(windDir.angleAvg_now()); Serial.print(F(" ADU\t"));
//	Serial.print(F("WindDirection = ")); Serial.print(windDir.heading()); Serial.println();
//}
