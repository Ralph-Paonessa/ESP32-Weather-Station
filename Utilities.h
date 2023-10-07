// Utilities.h

#ifndef _UTILITIES_h
#define _UTILITIES_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

/// <summary>
/// Exposes functions for processing weather data.
/// </summary>
namespace Utilities {

	/// <summary>
	/// Converts millibars to inches of mercury.
	/// </summary>
	/// <param name="p_mb">Pressure in millibars.</param>
	/// <returns>Pressure in inches of mercury.</returns>
	float pressure_inches(float p_mb);

	/// <summary>
	/// Returns air pressure adjusted to the equivalent sea level pressure.
	/// </summary>
	/// <param name="pressure">Absolute pressure (any units).</param>
	/// <param name="alt_m">Altitude, meters.</param>
	/// <param name="temp_C">Reading temperature, C.</param>
	/// <returns>Air pressure at sea level.</returns>
	float pressureAtSeaLevel(float pressure, float alt_m, float temp_C);

	/// <summary>
	/// Returns boiling point of water (F) as function of air pressure.
	/// </summary>
	/// <param name="pressure_mb">Air pressure, mbar</param>
	/// <returns> Boiling point of water (F).</returns>
	float waterBoilingPoint_F(float pressure_mb);

	/// <summary>
	/// Returns normalized insolation as percent of reference maximum.
	/// </summary>
	/// <param name="insol_mV">Raw insolation measurement, mV.</param>
	/// <param name="insol_ref">Reference insolation value, mV.</param>
	/// <returns>Insolation as percent of reference value.</returns>
	float insol_norm_pct(float insol_mV, float insol_ref);

	/// <summary>
	/// Returns Fahrenheit temp converted from Celcius.
	/// </summary>
	/// <param name="tempC">Temperature in Celcius</param>
	/// <returns>Fahrenheit temp converted from Celcius.</returns>
	float temperature_F(float tempC);

	/// <summary>
	/// Returns Celcius temp converted from Fahrenheit.
	/// </summary>
	/// <param name="tempF">Temperature in Fahrenheit</param>
	/// <returns>Celcius temp converted from Fahrenheit.</returns>
	float temperature_C(float tempF);

	/// <summary>
	/// Returns fan rpm from half-rotation count over 
	/// time period, for fan with 2 counts/rotation.
	/// </summary>
	/// <param name="countHalfRots">Count of half-rotations.</param>
	/// <param name="periodOfRotation">Period of rotations (sec).</param>
	/// <returns>Fan rpm.</returns>
	float fanRPM(int countHalfRots, float periodOfRotation);

	/// <summary>
	/// Returns "OK" or "ERROR" string from bool.
	/// </summary>
	/// <param name="val">Bool to parse.</param>
	/// <returns>"OK" if true, "ERROR" if false.</returns>
	String bool_OK_Error(bool val);

	/// <summary>
	/// Returns "OK" or "ERROR" from bool.
	/// </summary>
	/// <param name="val">Boolean to parse.</param>
	/// <returns>"true" or "false".</returns>
	String bool_true_false(bool val);

	/// <summary>
	/// Returns "Yes" or "No" from boolean.
	/// </summary>
	/// <param name="val">Boolean to parse.</param>
	/// <returns>"Yes" or "No".</returns>
	String bool_Yes_No(bool val);

}

#endif
