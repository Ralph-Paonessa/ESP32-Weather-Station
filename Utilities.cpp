// Functions useful to processing weather data.
// 
// 

#include "Utilities.h"

/// <summary>
/// Converts millibars to inches of mercury.
/// </summary>
/// <param name="p_mb">Pressure in millibars.</param>
/// <returns>Pressure in inches of mercury.</returns>
float Utilities::pressure_inches(float p_mb) {
	return p_mb / 33.864;
}

/// <summary>
/// Returns air pressure adjusted to the equivalent sea level pressure.
/// </summary>
/// <param name="pressure">Absolute pressure (any units).</param>
/// <param name="alt_m">Altitude, meters.</param>
/// <param name="temp_C">Reading temperature, C.</param>
/// <returns>Air pressure at sea level.</returns>
float Utilities::pressureAtSeaLevel(
	float pressure,
	float alt_m,
	float temp_C) {
	float temp_Kelvin = 273.15 + temp_C;
	// Adjust absolute pressure to sea level.
	float scaleFactor = pow(1 - ((0.0065 * alt_m) / (temp_Kelvin + (0.0065 * alt_m))), -5.257);
	return  pressure * scaleFactor;
}

/// <summary>
/// Returns boiling point of water (F) as function of air pressure.
/// </summary>
/// <param name="pressure_mb">Air pressure, mbar</param>
/// <returns> Boiling point of water (F).</returns>
float Utilities::waterBoilingPoint_F(float pressure_mb){
	return  49.161 * log(pressure_inches(pressure_mb)) + 44.932;
}

/// <summary>
/// Returns normalized insolation as percent of reference maximum.
/// </summary>
/// <param name="insol_mV">Raw insolation measurement, mV.</param>
/// <param name="insol_ref">Reference insolation value, mV.</param>
/// <returns>Insolation as percent of reference value.</returns>
float Utilities::insol_norm_pct(float insol_mV, float insol_ref) {
	return insol_mV / insol_ref * 100;	// Relative, percent.
}

/// <summary>
/// Returns Fahrenheit temp converted from Celcius.
/// </summary>
/// <param name="tempC">Temperature in Celcius</param>
/// <returns>Fahrenheit temp converted from Celcius.</returns>
float Utilities::temperature_F(float tempC) {
	return 1.8 * tempC + 32.;
}

/// <summary>
/// Returns Celcius temp converted from Fahrenheit.
/// </summary>
/// <param name="tempF">Temperature in Fahrenheit</param>
/// <returns>Celcius temp converted from Fahrenheit.</returns>
float Utilities::temperature_C(float tempF) {
	return (tempF - 32.) / 1.8;
}

/// <summary>
/// Returns fan rpm from half-rotation count over 
/// time period, for fan with 2 counts/rotation.
/// </summary>
/// <param name="countHalfRots">Count of half-rotations.</param>
/// <param name="periodOfRotation">Period of rotations (sec).</param>
/// <returns>Fan rpm.</returns>
float Utilities::fanRPM(int countHalfRots, float periodOfRotation) {
	// fan rot/min = (fan counts)/(FAN_PERIOD sec) * (1 rot / 2 counts) * (60 sec/min)
	return (countHalfRots / periodOfRotation / 2 * 60);
}

/// <summary>
/// Returns "OK" or "ERROR" string from bool.
/// </summary>
/// <param name="val">Bool to parse.</param>
/// <returns>"OK" if true, "ERROR" if false.</returns>
String Utilities::bool_OK_Error(bool val) {
	if (val) {
		return "OK";
	}
	else {
		return "ERROR";
	}
}

/// <summary>
/// Returns "OK" or "ERROR" from bool.
/// </summary>
/// <param name="val">Boolean to parse.</param>
/// <returns>"true" or "false".</returns>
String Utilities::bool_true_false(bool val) {
	if (val) {
		return "true";
	}
	else {
		return "false";
	}
}

/// <summary>
/// Returns "Yes" or "No" from boolean.
/// </summary>
/// <param name="val">Boolean to parse.</param>
/// <returns>"Yes" or "No".</returns>
String Utilities::bool_Yes_No(bool val) {
	if (val) {
		return "Yes";
	}
	else {
		return "No";
	}
}
