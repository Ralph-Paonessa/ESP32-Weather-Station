// SensorSimulate.h

#ifndef _SENSORSIMULATE_h
#define _SENSORSIMULATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "App_Settings.h"


/// <summary>
/// Exposes methods to generate simulated values.
/// </summary>
class SensorSimulate {

protected:

	float _val = 0;
	bool _isFirstComplete = false;
	bool _isSpiked = false;

	unsigned long _countCycles = 0;
	int _countSpikes = 0;
	bool _isReverse = false;


public:

	// Constructor
	SensorSimulate();

	/// <summary>
	/// Returns linearly-increasing value.
	/// </summary>
	/// <param name="initialVal">Initial value.</param>
	/// <param name="increment">Amount to increment on each iteration.</param>
	/// <returns></returns>
	float linear(float initialVal, float increment);

	/// <summary>
	/// Returns linearly-increasing value and periodically
	/// adds one or more spikes to the value.
	/// </summary>
	/// <param name="initialVal">Initial value.</param>
	/// <param name="increment">Amount to increment on each iteration.
	/// </param>
	/// <param name="spikeIncrement">Amount added by spike.</param>
	/// <param name="spikeStartCycle">
	/// Number of cycles at which spike occurs.</param>
	/// <param name="numSpikes">
	/// Number of spikes (must be greater than zero; default = 1).</param>
	/// <returns>Simulated value.</returns>
	float linear(
		float initialVal, 
		float increment, 
		float spikeIncrement, 
		unsigned long spikeStartCycle, 
		unsigned int numSpikes = 1);

	/// <summary>
	/// Returns sawtooth value and periodically adds
	/// one or more spikes to the value.
	/// </summary>
	/// <param name="initialVal">Initial value.</param>
	/// <param name="increment">Amount of change on each iteration.</param>
	/// <param name="max">Maximum value.</param>
	/// <returns>Value from a sawtooth pattern.</returns>
	float sawtooth(float initialVal, float increment, float max);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="initialVal"></param>
	/// <param name="increment"></param>
	/// <param name="max"></param>
	/// <param name="spikeIncrement"></param>
	/// <param name="spikeStartCycle"></param>
	/// <param name="numSpikes"></param>
	/// <returns></returns>
	float sawtooth(
		float initialVal, 
		float increment, 
		float max,
		float spikeIncrement,
		unsigned long spikeStartCycle,
		unsigned int numSpikes = 1);
};

#endif

