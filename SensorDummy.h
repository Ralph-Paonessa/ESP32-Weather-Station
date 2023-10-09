// SensorDummy.h

#ifndef _SENSORDUMMY_h
#define _SENSORDUMMY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


class SensorDummy{


protected:

	bool _isRisingValStarted = false;
	float _prevRisingVal = 0;


public:

	// Constructor
	SensorDummy();

	float randomValue(unsigned long seed, float max, float min);

	float risingVal(float initialVal, float increment);
};

#endif

