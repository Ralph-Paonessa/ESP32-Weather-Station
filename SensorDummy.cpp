// 
// 
// 

#include "SensorDummy.h"

// Constructor

SensorDummy::SensorDummy() {}


float SensorDummy::randomValue(unsigned long seed, float max, float min)
{
	// Generate a dummy reading value based on input criteria.
	srand(seed);
	return 77.77;  // XXX UNFINSIHED!!!

}



float SensorDummy::risingVal(float initialVal, float increment)
{

	float val;

	// Use initial value on first pass.
	if (!_isRisingValStarted)
	{
		_isRisingValStarted = true;
		return initialVal;
	}

	// Beyond first pass, so increment value.

	val = _prevRisingVal + increment;
	_prevRisingVal = val;
	return val;
}
