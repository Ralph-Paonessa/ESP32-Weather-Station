// 
// 
// 

#include "SensorSimulate.h"

// Constructor

SensorSimulate::SensorSimulate() {}


//float SensorSimulate::randomValue(unsigned long seed, float max, float min_today)
//{
//	// Generate a dummy reading value based on input criteria.
//	srand(seed);
//	return 77.77;  // XXX UNFINSIHED!!!
//
//}



/// <summary>
/// Returns linearly-increasing values.
/// </summary>
/// <param name="initialVal">Initial value.</param>
/// <param name="increment">Amount to increment on each iteration.</param>
/// <returns></returns>
float SensorSimulate::linear(float initialVal, float increment)
{
	// First iteration. Return starting.
	if (!_isLinearFirst) {
		_valLinear = increment;
		_isLinearFirst = true;		
		return _valLinear;		// first time
	}
	// After first iteration.
	_valLinear += increment;
	return _valLinear;
}

/// <summary>
/// 
/// </summary>
/// <param name="initialVal">Initial value.</param>
/// <param name="increment">Amount of change on each iteration.</param>
/// <param name="max">Maximum value.</param>
/// <returns></returns>
float SensorSimulate::sawtooth(float initialVal, float increment, float max)
{
	// First iteration. Return starting.
	if (_isSawtoothFirst) {
		_valSawtooth = initialVal;
		_isSawtoothFirst = false;
		return _valSawtooth;
	}
	// After first iteration.
	// Add increment.
	while (!_isSawtoothReverse)	{
		_valSawtooth += increment;	
		// Proceed until value reaches max.
		if (_valSawtooth >= max)
		{// Reverse direction.
			_isSawtoothReverse = !_isSawtoothReverse;
		}
		return _valSawtooth;
	}
	// Subtract increment.
	while (_isSawtoothReverse)	{
		_valSawtooth -= increment;	
		// Proceed until value returns to initial value.
		if (_valSawtooth <= initialVal)		{
			// Reverse direction.
			_isSawtoothReverse = !_isSawtoothReverse;
		}
		return _valSawtooth;
	}
}
