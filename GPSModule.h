/*
A library to handle GPS functions for a weather station GPS module.

Uses TinyGPS++ library to communicate with GPS module, and TimeLib
(Paul Stoffregen) to keep track of date and time obtined from GPS.

Because the GPS is used to set the application date and time using
TimeLib, the public time properties that the weather station uses
are defined here.

In addition, the SDCard library (which contains the status logging
functionality is used here so the status of gps syncing can be
logged to the SD card.
*/

// GPSModule.h

#ifndef _GPSMODULE_h
#define _GPSMODULE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <TinyGPSPlus.h>	// Mikal Hart - includes TinyGPS++.h
#include <TimeLib.h>		// Paul Stoffregen - Time

#include "App_Settings.h"
#include "SDCard.h"			// for data logging.
using namespace App_Settings;

/// <summary>
/// Structure that encapsulates GPS data.
/// </summary>
struct GPSData
{
	friend class GPSModule;

private:
	float _latitude;
	float _longitude;
	float _altitude;
	float _HDOP;
	unsigned int _satellites;
	unsigned long  _timeForSyncProcess_sec;		// Seconds elapsed while gps syncs.

public:

	/// <summary>
	/// GPS latitude at sync, degrees.
	/// </summary>
	/// <returns>Latitude, degrees.</returns>
	float latitude();

	/// <summary>
	/// GPS longitude at sync, degrees.
	/// </summary>
	/// <returns>Longitude, degrees.</returns>
	float longitude();

	/// <summary>
	/// GPS altitude at sync, meters.
	/// </summary>
	/// <returns>Altitude, meters.</returns>
	float altitude();

	/// <summary>
	/// GPS HDOP at sync.
	/// </summary>
	/// <returns>HDOP.</returns>
	float HDOP();

	/// <summary>
	/// Number of GPS satellites used at sync.
	/// </summary>
	/// <returns>Number of satellites.</returns>
	unsigned int satellites();

	/// <summary>
	/// Returns how long gps required to sync, seconds.
	/// </summary>
	/// <returns>Time required to sync, sec.</returns>
	unsigned long timeToSync_sec();
};

/// <summary>
/// Exposes methods to interact with a GPS module.
/// </summary>
class GPSModule {

private:
	bool _isSimulate = false;	// When true, simulate GPS fix data. (DEBUG)
	bool _isGpsSynced = false;	// flag when GPS data is synced to app
	int _countGpsCycles = 0;	// keep track of GPS cycles

	bool _isGpsReceiving = false;

	/// <summary>
	/// Set system time and date to GPS values.
	/// </summary>
	void syncSystemTimeToGPS();

	/// <summary>
	/// Save current GPS location data.
	/// </summary>
	void syncLocationToGPS();

	/// <summary>
	/// Returns true if the GPS date and time pass all validity tests.
	/// </summary>
	/// <returns>True if valid GPS date and time.</returns>
	bool isGpsDateTimeValid();

	/// <summary>
	/// Returns true if the GPS location data passes all validity tests.
	/// </summary>
	/// <returns>True if valid GPS location data.</returns>
	bool isGpsLocationValid();

	void syncSystemWithCurrentGpsData(unsigned long millisStart, int countGpsCycles);

	/// <summary>
	/// Log gps data.
	/// </summary>
	/// <param name="sdCard">SDCard instance for writing to log.</param>
	void logGpsData();

	void logData_Valid(int validCyclesCount);

	void logData_NotValid();

	/// <summary>
	/// Logs number of sentences that failed or passed checksum.
	/// </summary>
	void logData_checksumFailures();

	void logData_Valid_NotEnoughCycles(int countValidCycles);

	void logCurrentCycle();

	void logSyncIsComplete();

	static void gpsSmartDelay(unsigned long ms);

	void addDummyGpsData();

	void logSoftwareVersion();

	void logCountError(int countValidCycles);

public:

	/// <summary>
	/// Creates gps module instance.
	/// </summary>
	GPSModule();

	/// <summary>
	/// Structure to hold gps data.
	/// </summary>
	GPSData data;

	/// <summary>
	/// Creates GPS data connections.
	/// </summary>
	/// <param name="baudRate">Baud rate for GPS.</param>
	/// <param name="serialConfig">Serial configuration protocol.</param>
	/// <param name="rxPin">RX pin number on ESP32.</param>
	/// <param name="txPin">TX pin number on ESP32.</param>
	void begin(unsigned long baudRate,
		uint32_t serialConfig,
		int8_t rxPin,
		int8_t txPin);

	/// <summary>
	/// Returns true if GPS has received data.
	/// </summary>
	/// <returns>True if GPS has received data.</returns>
	bool isGpsReceiving();

	/// <summary>
	/// Returns true if adequate GPS time and 
	/// location data has been received and saved.
	/// </summary>
	/// <returns>True if gps data received and saved.</returns>
	bool isSynced();

	/// <summary>
	/// Number of GPS location updates.
	/// </summary>
	/// <returns>Number of location updates.</returns>
	unsigned int cyclesCount();

	/// <summary>
	/// Syncs gps time to the application once the gps is  
	/// providing sufficiently accurate time and location 
	/// data.
	/// </summary>
	/// <param name="sdCard">SDCard instance (for logging).</param>
	/// <param name="isSimulate">
	/// True to simulate gps sync and add dummy data 
	/// (default is false)
	/// </param>
	/// <returns>True if GPS sync success.</returns>
	bool syncToGPS(SDCard& sdCard, bool isSimulate);

	/// <summary>
	/// Returns current date and time string.
	/// </summary>
	/// <returns>Date and time as "yyyy-mm-dd hh:mm"</returns>
	String dateTime();

	/// <summary>
	/// Returns current time.
	/// (using TimeLib).
	/// </summary>
	/// <returns>Time as "hh:mm"</returns>
	String time();

	/// <summary>
	/// Returns GPS date as "MM/DD/YYYY".
	/// </summary>
	/// <returns>Date as "MM/DD/YYYY"</returns>
	String date_UTC_GPS();

	/// <summary>
	/// Returns UTC time from GPS.
	/// </summary>
	/// <returns>UTC time as "00:00:00".</returns>
	String time_UTC_GPS();

	/// <summary>
	/// Returns current day of the week as string (using TimeLib).
	/// </summary>
	/// <returns>Current day of the week.</returns>
	String dayName();

	/// <summary>
	/// Time zone offset in hours from UTC.
	/// </summary>
	/// <returns>Time zone offset in hours from UTC.</returns>
	int timeZoneOffset();

	/// <summary>
	/// Returns true if Daylight Time is set.
	/// </summary>
	/// <returns>True if Daylight Time is set</returns>
	bool isDaylightTime();
};

#endif
