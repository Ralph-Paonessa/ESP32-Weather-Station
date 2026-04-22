// Logging.h

#ifndef _LOGGING_h
#define _LOGGING_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "App_Settings.h"
using namespace App_Settings;
#include "DebugFlags.h"
#include "SDCard.h"


/// <summary>
/// Object that does something related to logging!
/// </summary>
class Logging {

private:

	SDCard _sd;
	String _gpsDateTime;
	bool _isDaylightTime;
	bool _wiFi_isConnected;
	int _gpsTimeZoneOffset;
	bool _isGood_SDCard;
	bool _isGood_LITTLEFS;
	bool _isGood_GPS;
	bool _isGood_Temp;
	bool _isGood_PRH;
	bool _isGood_UV;


public:

	// Constructor

	/// <summary>
	/// Initialize logging.
	/// </summary>
	/// <param name="offsetAngle">
	/// XXX
	/// </param>
	void Initialize(SDCard& sd);


	void SetValues(
		String gpsDateTime, 
		bool isDaylightTime, 
		bool wiFi_isConnected, 
		int gpsTimeZoneOffset, 
		bool _isGood_SDCard,
		bool _isGood_LITTLEFS,
		bool _isGood_GPS,
		bool _isGood_Temp,
		bool _isGood_PRH,
		bool _isGood_UV);

	// <summary>
	/// Logs summary of devices status.
	/// </summary>
	void logDeviceStatus();

	// <summary>
	/// Logs state of debug options.
	/// </summary>
	void logDebugStatus();

	// <summary>
	/// Logs space used by LittleFS.
	/// </summary>
	void logLittleFsSpaceUsage();

	// <summary>
	/// Logs the application settings to the STATUS file.
	/// </summary>
	void logApp_Settings();

};

#endif
