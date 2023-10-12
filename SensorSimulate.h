// SensorSimulate.h

#ifndef _SENSORSIMULATE_h
#define _SENSORSIMULATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


class SensorSimulate {


protected:

	float _valLinear = 0;
	bool _isLinearFirst = false;

	float _valSawtooth = 0;
	bool _isSawtoothFirst = true;
	bool _isSawtoothReverse = false;


public:

	// Constructor
	SensorSimulate();

	//float randomValue(unsigned long seed, float max, float min_today);

	float linear(float initialVal, float increment);

	float sawtooth(float initialVal, float increment, float max);
};

#endif

