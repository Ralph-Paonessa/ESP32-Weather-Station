// Testing.h

#ifndef _TESTING_h
#define _TESTING_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "WindSpeed.h"
#include "WindDirection.h"

#include <list>
using std::list;
#include "App_settings.h"
#include "ListFunctions.h"
#include "SensorData.h"
using namespace ListFunctions;
using namespace App_Settings;

//#include "TimeMonitor.h"

class Testing {

private:

	//TimeMonitor _timeMonLocal;

	float _dummyVal;
	int _countPeriods;
	////////int _countSpike;
	
public:

	void haltWithInfiniteLoop();

	// Tests WindSpeed.h wind directionCardinal handling.
	void windDirection(float angleStart, float angleIncrement, int cycles, float VANE_OFFSET);

	float testDummyReading(float seed, float increment1, float increment2, int periods, unsigned long count);

	int dummyRotations(float first, float last, float spike, int spikePeriod, int periods);

	//void test2();

	////void testList();
	void testStructList(long time);

	// Add a list of rotations to a WindSpeed object, and return elapsed seconds.
	float addDummyRotations(list<float> srcList, WindSpeed& speedObj, float rawPeriod);

	//void test();
		
	String readData();
};



#endif


