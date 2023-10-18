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
/// Returns linearly-increasing value and periodically
/// adds one or more spikes to the value.
/// </summary>
/// <param name="initialVal">Initial value.</param>
/// <param name="increment">Amount to increment on each iteration.</param>
/// <param name="spikeIncrement">Amount added by spike.</param>
/// <param name="spikeStartCycle">Number of cycles at which spike occurs.</param>
/// <param name="numSpikes">
/// Number of spikes (must be greater than zero; default = 1).
/// </param>
/// <returns>Linearly-increasing value with periodic spikes.</returns>
float SensorSimulate::linear(
	float initialVal,
	float increment,
	float spikeIncrement,
	unsigned long spikeStartCycle,
	unsigned int numSpikes) {
	float value = _val;		// Previous return value.
	if (numSpikes == 0) {
		numSpikes = 1;		// Can't be zero
	}
	_countCycles++;			// How many times have we returned a value?	
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
	if (_countCycles == spikeStartCycle && _countSpikes < numSpikes) {
		// Add a spike this cycle.
		_countSpikes++;
		_countCycles = 0;
		value += spikeIncrement;		// Add this spike.
		_val = value - spikeIncrement;	// Save _val for next cycle w/o increment.
	}
	return value;
}

/// <summary>
/// Returns value following a sawtooth pattern.
/// </summary>
/// <param name="initialVal">Initial value.</param>
/// <param name="increment">Amount of change on each iteration.</param>
/// <param name="max">Maximum value.</param>
/// <returns>Value from a sawtooth pattern.</returns>
float SensorSimulate::sawtooth(float initialVal, float increment, float max)
{
	float value = _val;		// Previous return value.
	// On first iteration, output initial value.
	if (!_isFirstComplete) {
		value = initialVal;
		_val = value;
		_isFirstComplete = true;
		return value;
	}
	// After first iteration output sawtooth.
	// Add increment.
	if (!_isReverse) {
		value += increment;
		_val = value;
		// When max reached, reverse direction.
		if (value >= max) {
			_isReverse = !_isReverse;
		}
	}
	// Subtract increment.
	if (_isReverse) {
		value -= increment;
		_val = value;
		// When initial value reached, reverse direction.
		if (value <= initialVal) {
			_isReverse = !_isReverse;
		}
	}
	return value;
}

/// <summary>
/// Returns sawtooth value and periodically adds
/// one or more spikes to the value.
/// </summary>
/// <param name="initialVal">Initial value.</param>
/// <param name="increment">Amount to increment on each iteration.</param>
/// <param name="max">Sawtooth maximum.</param>
/// <param name="spikeIncrement">Amount added by spike.</param>
/// <param name="spikeStartCycle">Number of cycles at which spike occurs.</param>
/// <param name="numSpikes">
/// Number of spikes (must be greater than zero; default = 1).
/// </param>
/// <returns>Sawtooth value.</returns>
float SensorSimulate::sawtooth(
	float initialVal,
	float increment,
	float max,
	float spikeIncrement,
	unsigned long spikeStartCycle,
	unsigned int numSpikes)
{
	//	float value = _val;		// Previous return value.
	if (numSpikes == 0) {
		numSpikes = 1;		// Can't be zero
	}
	_countCycles++;			// How many times have we returned a value?	

	float value = _val;		// Previous return value.
	// On first iteration, output initial value.
	if (!_isFirstComplete) {
		value = initialVal;
		_val = value;
		_isFirstComplete = true;
		return value;
	}
	// After first iteration output sawtooth.
	// Add increment.
	if (!_isReverse) {
		value += increment;
		_val = value;
		// When max reached, reverse direction.
		if (value >= max) {
			_isReverse = !_isReverse;
		}
	}
	// Subtract increment.
	if (_isReverse) {
		value -= increment;
		_val = value;
		// When initial value reached, reverse direction.
		if (value <= initialVal) {
			_isReverse = !_isReverse;
		}
	}
	// Periodically add spike to value.
	if (_countCycles == spikeStartCycle && _countSpikes < numSpikes) {
		// Add a spike this cycle.
		_countSpikes++;
		_countCycles = 0;
		if (_isReverse) {
			value -= spikeIncrement;		// Subtract this spike.
			_val = value + spikeIncrement;	// Save _val for next cycle w/o increment.
		}
		else {
			value += spikeIncrement;		// Add this spike.
			_val = value - spikeIncrement;	// Save _val for next cycle w/o increment.
		}
	}
	return value;
}
