// 
// 
// 

#include "Logging.h"

#include "App_Settings.h"
#include "DebugFlags.h"
#include "Utilities.h"
#include "SDCard.h"
#include <WString.h>

void Logging::Initialize(SDCard& sd)
{
	_sd = sd;
}

void Logging::SetValues(
	String gpsDateTime,
	bool isDaylightTime,
	bool wiFi_isConnected,
	int gpsTimeZoneOffset,
	bool isGood_SDCard,
	bool isGood_LITTLEFS,
	bool isGood_GPS,
	bool isGood_Temp,
	bool isGood_PRH,
	bool isGood_UV)
{
	_gpsDateTime = gpsDateTime;
	_isDaylightTime = isDaylightTime;
	_wiFi_isConnected = wiFi_isConnected;
	_gpsTimeZoneOffset = gpsTimeZoneOffset;
	_isGood_SDCard = _isGood_SDCard;
	_isGood_LITTLEFS = isGood_LITTLEFS;
	_isGood_GPS = isGood_GPS;
	_isGood_Temp = isGood_Temp;
	_isGood_PRH = isGood_PRH;
	_isGood_UV = isGood_UV;
}

// <summary>
/// Logs summary of devices status.
/// </summary>
void Logging::logDeviceStatus() {
	String msg = "DEVICE STATUS REPORT:";
	_sd.logStatus(msg, _gpsDateTime);

	msg = "WiFi connected: ";
	msg += Utilities::bool_OK_Error(_wiFi_isConnected);
	_sd.logStatus_indent(msg);

	msg = "SD card: ";
	msg += Utilities::bool_OK_Error(_isGood_SDCard);
	_sd.logStatus_indent(msg);

	msg = "LittleFS flash file system: ";
	msg += Utilities::bool_OK_Error(_isGood_LITTLEFS);
	_sd.logStatus_indent(msg);

	msg = "GPS module - ";
	msg += Utilities::bool_OK_Error(_isGood_GPS);
	_sd.logStatus_indent(msg);

	msg = "Dallas temperature sensor: ";
	msg += Utilities::bool_OK_Error(_isGood_Temp);
	_sd.logStatus_indent(msg);

	msg = "Radiation shield fan: ";
	msg += "???";
	//msg += bool_OK_Error(_isGood_fan);
	_sd.logStatus_indent(msg);

	msg = "Pressure & RH sensor: ";
	msg += Utilities::bool_OK_Error(_isGood_PRH);
	_sd.logStatus_indent(msg);

	msg = "Insolation sensor: ";
	msg += "???";
	_sd.logStatus_indent(msg);

	msg = "UV sensor: ";
	msg += Utilities::bool_OK_Error(_isGood_UV);
	_sd.logStatus_indent(msg);

	msg = "Wind direction sensor: ";
	msg += "???";
	_sd.logStatus_indent(msg);

	msg = "Wind speed sensor: ";
	msg += "???";
	_sd.logStatus_indent(msg);

	msg = "Time zone offset from UTC: ";
	msg += _gpsTimeZoneOffset;
	_sd.logStatus_indent(msg);

	msg = "Is Daylight Time: : ";
	msg += Utilities::bool_true_false(_isDaylightTime);
	_sd.logStatus_indent(msg);
}

/// <summary>
/// Logs state of debug options.
/// </summary>
void Logging::logDebugStatus() {
#if defined(VM_DEBUG)
	_sd.logStatus("Compiled as DEBUG BUILD");
#else
	_sd.logStatus("Compiled as RELEASE BUILD");
	Serial.print("Compiled as RELEASE BUILD\n");
#endif
	_sd.logStatus(LINE_SEPARATOR);
	_sd.logStatus("DEBUG FLAGS SET:");
	if (_isDEBUG_BypassGPS) {
		_sd.logStatus_indent("BYPASS GPS");
	}
	if (_isDEBUG_BypassWifi) {
		_sd.logStatus_indent("BYPASS WIFI");
	}
	if (_isDEBUG_ListLittleFS) {
		_sd.logStatus_indent("LIST LittleFS CONTENTS");
	}
	if (_isDEBUG_BypassSDCard) {
		Serial.println("BYPASS SD CARD");	// Can't log to SD!
	}
	if (_isDEBUG_BypassWebServer) {
		_sd.logStatus_indent("BYPASS WEB SERVER");
	}
	if (_isDEBUG_run_test_in_setup) {
		_sd.logStatus_indent("RUN TEST CODE IN SETUP");
	}
	if (_isDEBUG_run_test_in_loop) {
		_sd.logStatus_indent("RUN TEST CODE IN LOOP");
	}
	if (_isDEBUG_addDummyDataLists) {
		_sd.logStatus_indent("ADD DUMMY DATA");
	}
	if (_isDEBUG_simulateSensorReadings) {
		_sd.logStatus_indent("USE DUMMY DATA FOR SENSOR READINGS");
	}
	if (_isDEBUG_simulateWindReadings) {
		_sd.logStatus_indent("USE DUMMY DATA FOR WIND READINGS");
	}
	if (_isDEBUG_AddDelayInLoop) {
		String msg = String(_LOOP_DELAY_DEBUG_ms);
		msg += " ms DELAY ADDED IN LOOP";
		_sd.logStatus_indent(msg);
	}
	_sd.logStatus_indent("End of debug flags");
	_sd.logStatus(LINE_SEPARATOR);
}

/// <summary>
/// Logs space used by LittleFS.
/// </summary>
void Logging::logLittleFsSpaceUsage() {
	String msg = "LittleFS Total space: ";
	msg += String(LittleFS.totalBytes() / 1000.);
	msg += " KB";
	_sd.logStatus_indent(msg);
	msg = "LittleFS space used: ";
	msg += String(LittleFS.usedBytes() / 1000.);
	msg += " KB";
	_sd.logStatus_indent(msg);
	msg = "LittleFS space available: ";
	msg += String((LittleFS.totalBytes() - LittleFS.usedBytes()) / 1000.);
	msg += " KB";
	_sd.logStatus_indent(msg);
}

/// <summary>
/// Logs the application settings to the STATUS file.
/// </summary>
void Logging::logApp_Settings() {
	String msg = "APP SETTINGS:";
	_sd.logStatus(msg);
	msg = "BASE_PERIOD_SEC: " + String(BASE_PERIOD_SEC);
	msg += " sec";
	_sd.logStatus_indent(msg);
	msg = "GPS_CYCLES_FOR_SYNC: " + String(GPS_CYCLES_FOR_SYNC);
	_sd.logStatus_indent(msg);
	msg = "GPS_CYCLES_COUNT_MAX: " + String(GPS_CYCLES_COUNT_MAX);
	_sd.logStatus_indent(msg);
	msg = "GPS_DELAY_BETWEEN_CYCLES: " + String(GPS_CYCLE_DELAY_SEC);
	_sd.logStatus_indent(msg);
	msg = "GPS_MAX_ALLOWED_HDOP: " + String(GPS_MAX_ALLOWED_HDOP);
	_sd.logStatus_indent(msg);
	msg = "UTC_OFFSET_HOURS: " + String(UTC_OFFSET_HOURS);
	_sd.logStatus_indent(msg);
	msg = "IS_DAYLIGHT_TIME: " + String(IS_DAYLIGHT_TIME);
	_sd.logStatus_indent(msg);
	msg = "WIFI_CONNECT_TIMEOUT_SEC: " + String(WIFI_CONNECT_TIMEOUT_SEC);
	_sd.logStatus_indent(msg);
	msg = "LOGFILE_PATH_DATA: " + String(LOGFILE_PATH_DATA);
	_sd.logStatus_indent(msg);
	msg = "LOGFILE_PATH_STATUS: " + String(LOGFILE_PATH_STATUS);
	_sd.logStatus_indent(msg);
	msg = "INSOL_REFERENCE_MAX: " + String(INSOL_REFERENCE_MAX);
	_sd.logStatus_indent(msg);
	msg = "DAVIS_SPEED_CAL_FACTOR: " + String(DAVIS_SPEED_CAL_FACTOR);
	_sd.logStatus_indent(msg);
	msg = "VANE_OFFSET: " + String(VANE_OFFSET);
	_sd.logStatus_indent(msg);
	msg = "WIND_DIRECTION_SPEED_THRESHOLD: " + String(WIND_DIRECTION_SPEED_THRESHOLD);
	_sd.logStatus_indent(msg);
	msg = "LOOP_TIME_WARNING_THRESHOLD_MS: " + String(LOOP_TIME_WARNING_THRESHOLD_MS);
	_sd.logStatus_indent(msg);
}
