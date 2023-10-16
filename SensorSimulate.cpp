// 
// 
// 

#include "SensorSimulate.h"


// Constructor

SensorSimulate::SensorSimulate() {}


/// <summary>
/// Returns linearly-increasing value.
/// </summary>
/// <param name="initialVal">Initial value.</param>
/// <param name="increment">Amount to increment on each iteration.</param>
/// <returns></returns>
float SensorSimulate::linear(float initialVal, float increment)
{
	// First iteration. Return starting.
	if (!_isFirstComplete) {
		_val = initialVal;
		_isFirstComplete = true;
		return _val;		// first time
	}
	// After first iteration.
	_val += increment;
	return _val;
}

/// <summary>
/// Returns linearly-increasing value, but adds
/// one or more spikes to the value.
/// </summary>
/// <param name="initialVal">Initial value.</param>
/// <param name="increment">Amount to increment on each iteration.</param>
/// <param name="spikeIncrement">Amount added by spike.</param>
/// <param name="spikeStartCycle">Number of cycles at which spike occurs.</param>
/// <param name="numSpikes">
/// Number of spikes (must be greater than zero; default = 1).
/// </param>
/// <returns>Simulated value.</returns>
float SensorSimulate::linear_spike(
	float initialVal,
	float increment,
	float spikeIncrement,
	unsigned long spikeStartCycle,
	int numSpikes
)
{
	float value = _val;		// Previous return value.
	if (numSpikes == 0) {
		numSpikes = 1;	// Can't be zero
	}
	_countCycles++;		// How many times have we returned a value?	
	if (!_isFirstComplete) {
		// On first iteration, return initial value.
		value = initialVal;
		_val = value;
		_isFirstComplete = true;
	}
	else {
		// After first iteration, increase the value.
		value += increment;
		_val = value;
	}
	if (_countCycles == spikeStartCycle && _countSpikes < numSpikes)
	{
		// Add a spike this cycle.
		_countSpikes++;
		_countCycles = 0;
		value += spikeIncrement;		// Add this spike.
		_val = value - spikeIncrement;	// Save _val for next cycle w/o increment.
	}
	return value;
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
	if (!_isFirstComplete) {
		_val = initialVal;
		_isFirstComplete = true;
		return _val;
	}
	// After first iteration.
	// Add increment.
	while (!_isReverse) {
		_val += increment;
		// Proceed until value reaches max.
		if (_val >= max)
		{
			_isReverse = !_isReverse;	// Reverse direction.
		}
		return _val;
	}
	// Subtract increment.
	while (_isReverse) {
		_val -= increment;
		// Proceed until value returns to initial value.
		if (_val <= initialVal) {
			// Reverse direction.
			_isReverse = !_isReverse;
		}
		return _val;
	}
}
