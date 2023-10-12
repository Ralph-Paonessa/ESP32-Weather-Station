/*
A library to handle GPS functions for a weather station GPS module.

Uses TinyGPS++ library to communicate with GPS module, and TimeLib
(Paul Stoffregen) to keep track of date and time obtined from GPS.

*/

#include "GPSModule.h"

TinyGPSPlus _tinyGPS;				// TinyGPS++ object.
HardwareSerial _serialGPS(2);		// Allowed values 0, 1, 2?

SDCard _sdCard;		// SDCard object for data logging.

/// <summary>
/// Initialize GPSModule object that interacts with a GPS module.
/// </summary>
GPSModule::GPSModule() {}

/// <summary>
/// Sets GPS data connections.
/// </summary>
/// <param name="baudRate">Baud rate for GPS.</param>
/// <param name="serialConfig">Serial configuration protocol.</param>
/// <param name="rxPin">RX pin no. on ESP32.</param>
/// <param name="txPin">TX pin no. on ESP32.</param>
/// <param name="isBypass">
/// Set true to simulate gps sync and add dummy data (default is false).
/// </param>
void GPSModule::begin(
	unsigned long baudRate,
	uint32_t serialConfig,
	int8_t rxPin,
	int8_t txPin) {
	// Initialize GPS UART communication.
	/*
	 The format for setting a serial port is as follows:
	 SerialObject.begin(baud-rate, protocol, RX pin, TX pin);
	 (Wrong baud rate gives serial garbage.)
	*/
	_serialGPS.begin(baudRate, serialConfig, rxPin, txPin);
}

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
void GPSModule::syncToGPS(SDCard& sdCard, bool isSimulate) {
	unsigned long timeStart = millis();
	_sdCard = sdCard;		// SDCard object for data logging.
	_isSimulate = isSimulate;

	// Allow gps to be bypassed when flag  is set.
	if (_isSimulate) {
		// Pretend gps is synced.
		_isGpsSynced = true;
		addDummyGpsData();
		_sdCard.logStatus("BYPASSING GPS WITH DUMMY DATA.", millis());
		return;		// Bypass sync.
	}

	// Sync system to GPS.
	// Output progress in syncing.
	_sdCard.logStatus("Begin search for GPS signal.", millis());
	bool isFirstTime = false;	// Flag for logging status on first pass.
	unsigned int countValidCycles = 0;
	String msg;

	// Loop while data is in the serial buffer and until GPS syncs.
	while (_serialGPS.available() > 0 || !_isGpsSynced) {
		// Log this message the first time in the loop.
		if (!isFirstTime) {
			isFirstTime = true;
			_sdCard.logStatus("Receiving GPS data.", millis());
		}

		// Need GPS_CYCLES_FOR_SYNC cyles of valid gps data.
		// While GPS is not synced and GPS is encoding data.
		while (!_isGpsSynced || _tinyGPS.encode(_serialGPS.read()))
		{
			// A new GPS sentence was encoded.
			_countGpsCycles++;
			logCurrentCycle();

			// Does the data pass our validity tests?
			if (isGpsDataValid())
			{
				// VALID DATA.
				// Must have valid data for GPS_CYCLES_FOR_SYNC
				// *consecutive* cycles.
				countValidCycles++;
				logData_Valid(countValidCycles);
				// Error check.
				if (countValidCycles > GPS_CYCLES_FOR_SYNC) {
					logCountError(countValidCycles);
				}
				// Data is valid, but ...
				// Do we have enough consecutive cycles of valid data?
				if (countValidCycles == GPS_CYCLES_FOR_SYNC) {
					/***************   SUCCESS!!   **************/
					// Sync system time and location with the GPS. 
					syncSystemWithCurrentGpsData(timeStart, _countGpsCycles);
					_isGpsSynced = true;	// flag GPS synced and we're finished.
				}
				else
				{
					// Not enough valid cycles yet.
					logData_Valid_NotEnoughCycles(countValidCycles);
				}
			}	// (isGpsDataValid())
			else
			{
				// INVALID DATA.
				logData_NotValid();
				countValidCycles = 0;	// Reset valid cycles.
			}	// (isGpsDataValid())

			// If not synced, wait between cycles.
			if (!_isGpsSynced) {
				// Wait but keep receiving GPS data.
				gpsSmartDelay(GPS_DELAY_BETWEEN_CYCLES);
			}
		}	// while (!_isGpsSynced || _tinyGPS.encode(_serialGPS.read()))
	}		// while (_serialGPS.available() > 0)
	// GPS sync successful, so wrap up and return.
	logSyncIsComplete();
}

/// <summary>
/// Uses dummy data as current time and altitude.
/// </summary>
void GPSModule::addDummyGpsData() {
	data._altitude = 1234.;	// Dummy altitude (need for P at sea level)
	setTime(GPS_DUMMY_HOUR, 
		GPS_DUMMY_MIN, 
		GPS_DUMMY_SEC, 
		GPS_DUMMY_DAY, 
		GPS_DUMMY_MONTH, 
		GPS_DUMMY_YEAR);
}

/// <summary>
/// Verifies that the valid cycle count doesn't 
/// exceed the number required for sync.
/// </summary>
/// /// <param name="countValidCycles">Number of valid cycles.</param>
void GPSModule::logCountError(int countValidCycles) {
	String msg = "ERROR: countValidCycles = ";
	msg += String(countValidCycles);
	msg += " exceeds GPS_CYCLES_FOR_SYNC = ";
	msg += String(GPS_CYCLES_FOR_SYNC);
	_sdCard.logStatus(msg, millis());
}

/// <summary>
/// Logs that the GPS data meets our validity tests.
/// </summary>
/// <param name="countValidCycles">Number of valid cycles.</param>
void GPSModule::logData_Valid(int countValidCycles) {
	String msg = "GPS data: VALID DATA at ";
	msg += String(countValidCycles);
	msg += " of " + String(GPS_CYCLES_FOR_SYNC);
	msg += " cycles required to sync).";
	_sdCard.logStatus(msg, millis());
}

/// <summary>
/// Logs that the GPS data fails our validity tests.
/// </summary>
void GPSModule::logData_NotValid() {
	String msg = "GPS data: INVALID data after ";
	msg += String(_countGpsCycles);
	msg += " total cycles.";
	_sdCard.logStatus(msg, millis());
}

/// <summary>
/// Logs that the GPS data fails our validity tests.
/// </summary>
/// <param name="countValidCycles">Number of valid cycles.</param>
void GPSModule::logData_Valid_NotEnoughCycles(int countValidCycles) {
	String msg = "GPS sync still requires: ";
	msg += String(GPS_CYCLES_FOR_SYNC - countValidCycles);
	msg += " more consecutive valid cycles to sync.";
	_sdCard.logStatus_indent(msg);
}

/// <summary>
/// Logs this cyle of gps data retrieval.
/// </summary>
void GPSModule::logCurrentCycle() {
	_sdCard.logStatus(LINE_SEPARATOR);
	String msg = "GPS LOCATION UPDATE CYCLE ";
	msg += String(_countGpsCycles);
	_sdCard.logStatus(msg, millis());
	logGpsData();	// Log gps readings for this update cycle.
}

/// <summary>
/// GPS sync successful, so wraps up and returns.
/// </summary>
void GPSModule::logSyncIsComplete() {
	_sdCard.logStatus(LINE_SEPARATOR);
	_sdCard.logStatus(LINE_SEPARATOR);
	_sdCard.logStatus("[   Reminder: GPS HAS NOT BEEN POWERED DOWN.   ]");
	_sdCard.logStatus("GPS sync complete.", millis());
	String msg = "Local date and time " + dateTime();
	_sdCard.logStatus(msg);
	msg = " Using offset from UTC = " + String(UTC_OFFSET_HOURS) + " hr.";
	if (IS_DAYLIGHT_TIME) {
		msg += " Adjusted + 1 hr for Daylight Time.";
	}
	else {
		msg += " Using Standard Time.";
	}
	_sdCard.logStatus(msg, millis());
	_sdCard.logStatus(LINE_SEPARATOR);
}

/// <summary>
/// Returns true if the GPS data passes all validity tests.
/// </summary>
/// <returns>True if valid GPS data.</returns>
bool GPSModule::isGpsDataValid() {
	bool isValid = false;
	if (
		_tinyGPS.satellites.value() >= GPS_SATELLITES_REQUIRED
		&& _tinyGPS.date.isValid()
		&& _tinyGPS.location.isValid()
		&& _tinyGPS.altitude.isValid()
		&& _tinyGPS.hdop.value() / 100. <= GPS_MAX_ALLOWED_HDOP
		)
	{
		isValid = true;
	}
	return isValid;
}

/// <summary>
/// Sets the system time and location to the current GPS data.
/// </summary>
/// <param name="millisStart">Mills when the sync operation started.</param>
/// <param name="countGpsCycles">Total number of GPS cycles.</param>
/// <param name="sdCard">SDCard object for logging.</param>
void GPSModule::syncSystemWithCurrentGpsData(unsigned long millisStart, int countGpsCycles) {
	String msg = "GPS valid data criteria met after ";
	msg += String(countGpsCycles) + " cycles.";
	_sdCard.logStatus(msg, millis());
	data._timeForSyncProcess_sec = (millis() - millisStart) / 1000.;
	syncSystemTimeToGPS();
	syncLocationToGPS();
	_sdCard.logStatus("GPS data: COMPLETE. Sync GPS data.", millis());
}

/// <summary>
/// This custom version of delay() ensures that the
/// gps object is being "fed".
/// </summary>
/// <param name="delay">Delay, sec.</param>
void GPSModule::gpsSmartDelay(unsigned long delay) {
	unsigned long start = millis();
	do {
		while (_serialGPS.available()) {
			_tinyGPS.encode(_serialGPS.read());
		}
	} while (millis() - start < delay * 1000);
}

/// <summary>
/// Saves current GPS location data.
/// </summary>
void GPSModule::syncLocationToGPS() {
	// Keep the current GPS data.
	data._latitude = _tinyGPS.location.lat();
	data._longitude = _tinyGPS.location.lng();
	data._altitude = _tinyGPS.altitude.meters();
	data._satellites = _tinyGPS.satellites.value();
	data._HDOP = _tinyGPS.hdop.value();
}

/// <summary>
/// Writes to status log the values of parameters 
/// read from the gps and whether they're valid.
/// </summary>
/// <param name="sdCard">SDCard instance for writing to log file.</param>
void GPSModule::logGpsData() {
	// Location.
	String msg;
	if (_tinyGPS.location.isValid()) {
		msg = "Latitude\t" + String(_tinyGPS.location.lat(), 6);
		_sdCard.logStatus_indent(msg);
		msg = "Longitude\t" + String(_tinyGPS.location.lng(), 6);
		_sdCard.logStatus_indent(msg);
	}
	else {
		_sdCard.logStatus_indent("Location INVALID");
	}
	// Altitude in meters (double)
	if (_tinyGPS.altitude.isValid()) {
		msg = "Altitude\t" + String(_tinyGPS.altitude.meters()) + "m";
		_sdCard.logStatus_indent(msg);
	}
	else {
		_sdCard.logStatus_indent("Altitude INVALID");
	}
	// Date.
	msg = "Date\t\t";
	if (_tinyGPS.date.isValid()) {
		msg += date_UTC_GPS();
		_sdCard.logStatus_indent(msg);
	}
	else {
		_sdCard.logStatus_indent("INVALID");
	}
	// Time.
	msg = "Time\t\t";
	if (_tinyGPS.time.isValid()) {
		msg += time_UTC_GPS() + " UTC";
		_sdCard.logStatus_indent(msg);
	}
	else {
		_sdCard.logStatus_indent("INVALID");
	}
	// Number of satellites in use (u32)
	msg = "Satellites\t" + String(_tinyGPS.satellites.value());
	_sdCard.logStatus_indent(msg);

	// Horizontal DOP (Dilution of Precision) (100ths-i32)
	msg = "Horizontal DOP\t" + String(_tinyGPS.hdop.value() / 100., 2);
	_sdCard.logStatus_indent(msg);
}

/// <summary>
/// Sync time and date to GPS.
/// </summary>
void GPSModule::syncSystemTimeToGPS() {
	// Use TimLib setTime:
	// setTime(hours, minutes, seconds, days, months, years)
	setTime(_tinyGPS.time.hour(), _tinyGPS.time.minute(), _tinyGPS.time.second(),
		_tinyGPS.date.day(), _tinyGPS.date.month(), _tinyGPS.date.year());
	// Convert from UTC to current time zone.
	adjustTime(UTC_OFFSET_HOURS * 3600);
	// Add back 1 hour if daylight time.
	if (IS_DAYLIGHT_TIME) {
		adjustTime(3600);
	}
}

/// <summary>
/// Returns true if adequate GPS time and 
/// location data has been received and saved.
/// </summary>
/// <returns>True if gps data received and saved.</returns>
bool GPSModule::isSynced()
{
	return _isGpsSynced;
}

/// <summary>
/// Number of GPS cycles.
/// </summary>
/// <returns>Number of cycles.</returns>
unsigned int GPSModule::cyclesCount()
{
	return _countGpsCycles;
}

/// <summary>
/// Returns GPS date as "MM/DD/YYYY".
/// </summary>
/// <returns>"MM/DD/YYYY" from GPS.</returns>
String GPSModule::date_UTC_GPS() {
	String s = "";
	s += _tinyGPS.date.month();
	s += "-";
	s += _tinyGPS.date.day();
	s += "-";
	s += _tinyGPS.date.year();
	return s;
}

/// <summary>
/// Returns raw GPS time as "00:00:00".
/// </summary>
/// <returns>GPS time "00:00:00".</returns>
String GPSModule::time_UTC_GPS() {
	String s = "";
	if (_tinyGPS.time.hour() < 10) {
		s += "0";
	}
	s += _tinyGPS.time.hour();
	s += ":";
	if (_tinyGPS.time.minute() < 10) {
		s += "0";
	}
	s += _tinyGPS.time.minute();
	s += ":";
	if (_tinyGPS.time.second() < 10) {
		s += "0";
	}
	s += _tinyGPS.time.second();
	return s;
}

/// <summary>
/// Returns time zone offset from UTC
/// </summary>
/// <returns></returns>
int GPSModule::timeZoneOffset() {
	return UTC_OFFSET_HOURS;
}

/// <summary>
/// Returns true if initialized to daylight time.
/// </summary>
/// <returns></returns>
bool GPSModule::isDaylightTime() {
	return IS_DAYLIGHT_TIME;
}

/// <summary>
/// Returns string "yyyy-mm-dd hh:mm" from 
/// current date and time (using TimeLib).
/// </summary>
/// <returns></returns>
String GPSModule::dateTime() {
	if (!_isGpsSynced) {
		return String(millis() / 1000., 2) + "s";	// Date not yet known.
	}
	time_t t = now(); // Hold current time.
	String s = String(year(t)) + "-";
	// Month.
	if (month(t) < 10) { s += "0"; }
	s += String(month(t)) + "-";
	// Day.
	if (day(t) < 10) { s += "0"; }
	s += String(day(t)) + " ";
	// Hour.
	if (hour(t) < 10) { s += "0"; }
	s += String(hour(t));
	s += ":";
	// Minute.
	if (minute(t) < 10) { s += "0"; }
	s += String(minute(t));
	return s;
}

/// <summary>
/// Returns current system time as "hh:mm".
/// </summary>
/// <returns>Current time "hh:mm"</returns>
String GPSModule::time() {
	if (!_isGpsSynced) {
		return String(now()) + "s";
	}
	String s = "";
	time_t t = now(); // Save current time.
	// Time
	if (hour(t) < 10) {      // Add leading zero.
		s += "0" + String(hour(t)) + ":";
	}
	else {
		s += String(hour(t)) + ":";
	}
	if (minute(t) < 10) {    // Add leading zero.
		s += "0" + String(minute(t));
	}
	else {
		s += String(minute(t));
	}
	return s;
}

/// <summary>
/// Returns current day of the week as string.
/// </summary>
/// <returns></returns>
String GPSModule::dayString() {
	return dayStr(day());
}

/// <summary>
/// Latitude when GPS was synced to application.
/// </summary>
/// <returns></returns>
float GPSData::latitude() {
	return _latitude;
}

/// <summary>
/// Longitude when GPS was synced to application.
/// </summary>
/// <returns></returns>
float GPSData::longitude() {
	return _longitude;
}
/// <summary>
/// Altitude (meters) when GPS was synced to application.
/// </summary>
/// <returns></returns>
float GPSData::altitude() {
	return _altitude;
}
/// <summary>
/// Horizontal Degree of Precision when GPS was 
/// synced to application.
/// </summary>
/// <returns></returns>
float GPSData::HDOP() {
	return _HDOP;
}
/// <summary>
/// Number of satellites used when GPS was synced 
/// to application.
/// </summary>
/// <returns></returns>
unsigned int GPSData::satellites() {
	return _satellites;
}

unsigned long GPSData::timeToSync_sec()
{
	return _timeForSyncProcess_sec;
}
