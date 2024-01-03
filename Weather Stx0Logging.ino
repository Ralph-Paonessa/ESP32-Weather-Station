//// %%%%%%%%%%   STATUS FLAGS FOR DEVICES   %%%%%%%%%%%%%%%%
//bool _isGood_Temp = false;
//bool _isGood_PRH = false;
//bool _isGood_UV = false;
//bool _isGood_IR = false;
//bool _isGood_WindDir = false;
//bool _isGood_WindSpeed = false;
////bool _isGood_GPS = false;
//bool _isGood_PMS = false;
////bool _isGood_SDCard = false;
//bool _isGood_Solar = false;
//bool _isGood_LITTLEFS = false;
//bool _isGood_fan = false;
//// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


/// <summary>
/// Logs summary of devices status.
/// </summary>
void logDeviceStatus() {
	String msg = "DEVICE STATUS REPORT:";
	sd.logStatus(msg, gps.dateTime());

	msg = "WiFi connected: ";
	msg += bool_OK_Error(WiFi.isConnected());
	sd.logStatus_indent(msg);

	msg = "SD card: ";
	msg += bool_OK_Error(_isGood_SDCard);
	sd.logStatus_indent(msg);

	msg = "LittleFS flash file system: ";
	msg += bool_OK_Error(_isGood_LITTLEFS);
	sd.logStatus_indent(msg);

	msg = "GPS module - ";
	msg += bool_OK_Error(_isGood_GPS);
	sd.logStatus_indent(msg);

	msg = "Dallas temperature sensor: ";
	msg += bool_OK_Error(_isGood_Temp);
	sd.logStatus_indent(msg);

	msg = "Radiation shield fan: ";
	msg += "???";
	//msg += bool_OK_Error(_isGood_fan);
	sd.logStatus_indent(msg);

	msg = "Pressure & RH sensor: ";
	msg += bool_OK_Error(_isGood_PRH);
	sd.logStatus_indent(msg);

	msg = "Insolation sensor: ";
	msg += "???";
	sd.logStatus_indent(msg);

	msg = "UV sensor: ";
	msg += bool_OK_Error(_isGood_UV);
	sd.logStatus_indent(msg);

	msg = "Wind direction sensor: ";
	msg += "???";
	sd.logStatus_indent(msg);

	msg = "Wind speed sensor: ";
	msg += "???";
	sd.logStatus_indent(msg);

	msg = "Time zone offset from UTC: ";
	msg += gps.timeZoneOffset();
	sd.logStatus_indent(msg);

	msg = "Is Daylight Time: : ";
	msg += bool_true_false(gps.isDaylightTime());
	sd.logStatus_indent(msg);
}

/// <summary>
/// Logs state of debug options.
/// </summary>
void logDebugStatus() {
#if defined(VM_DEBUG)
	sd.logStatus("Compiled as DEBUG BUILD");
#else
	sd.logStatus("Compiled as RELEASE BUILD");
	Serial.print("Compiled as RELEASE BUILD\n");
#endif
	sd.logStatus(LINE_SEPARATOR);
	sd.logStatus("DEBUG FLAGS SET:");
	if (_isDEBUG_BypassGPS) {
		sd.logStatus_indent("BYPASS GPS");
	}
	if (_isDEBUG_BypassWifi) {
		sd.logStatus_indent("BYPASS WIFI");
	}
	if (_isDEBUG_ListLittleFS) {
		sd.logStatus_indent("LIST LittleFS CONTENTS");
	}
	if (_isDEBUG_BypassSDCard) {
		Serial.println("BYPASS SD CARD");	// Can't log to SD!
	}
	if (_isDEBUG_BypassWebServer) {
		sd.logStatus_indent("BYPASS WEB SERVER");
	}
	if (_isDEBUG_run_test_in_setup) {
		sd.logStatus_indent("RUN TEST CODE IN SETUP");
	}
	if (_isDEBUG_run_test_in_loop) {
		sd.logStatus_indent("RUN TEST CODE IN LOOP");
	}
	if (_isDEBUG_addDummyDataLists) {
		sd.logStatus_indent("ADD DUMMY DATA");
	}
	if (_isDEBUG_simulateSensorReadings) {
		sd.logStatus_indent("USE DUMMY DATA FOR SENSOR READINGS");
	}
	if (_isDEBUG_simulateWindReadings) {
		sd.logStatus_indent("USE DUMMY DATA FOR WIND READINGS");
	}
	if (_isDEBUG_AddDelayInLoop) {
		String msg = String(_LOOP_DELAY_DEBUG_ms);
		msg += " ms DELAY ADDED IN LOOP";
		sd.logStatus_indent(msg);
	}
	sd.logStatus_indent("End of debug flags");
	sd.logStatus(LINE_SEPARATOR);
}

/// <summary>
/// Logs space used by LittleFS.
/// </summary>
void logLittleFsSpaceUsage() {
	String msg = "LittleFS Total space: ";
	msg += String(LittleFS.totalBytes() / 1000.);
	msg += " KB";
	sd.logStatus_indent(msg);
	msg = "LittleFS space used: ";
	msg += String(LittleFS.usedBytes() / 1000.);
	msg += " KB";
	sd.logStatus_indent(msg);
	msg = "LittleFS space available: ";
	msg += String((LittleFS.totalBytes() - LittleFS.usedBytes()) / 1000.);
	msg += " KB";
	sd.logStatus_indent(msg);
}



/// <summary>
/// LogS the application settings to the STATUS file.
/// </summary>
void logApp_Settings() {
	String msg = "APP SETTINGS:";
	sd.logStatus(msg);
	msg = "BASE_PERIOD_SEC: " + String(BASE_PERIOD_SEC);
	msg += " sec";
	sd.logStatus_indent(msg);
	msg = "GPS_CYCLES_FOR_SYNC: " + String(GPS_CYCLES_FOR_SYNC);
	sd.logStatus_indent(msg);
	msg = "GPS_CYCLES_COUNT_MAX: " + String(GPS_CYCLES_COUNT_MAX);
	sd.logStatus_indent(msg);
	msg = "GPS_DELAY_BETWEEN_CYCLES: " + String(GPS_CYCLE_DELAY_SEC);
	sd.logStatus_indent(msg);
	msg = "GPS_MAX_ALLOWED_HDOP: " + String(GPS_MAX_ALLOWED_HDOP);
	sd.logStatus_indent(msg);
	msg = "UTC_OFFSET_HOURS: " + String(UTC_OFFSET_HOURS);
	sd.logStatus_indent(msg);
	msg = "IS_DAYLIGHT_TIME: " + String(IS_DAYLIGHT_TIME);
	sd.logStatus_indent(msg);
	msg = "WIFI_CONNECT_TIMEOUT_SEC: " + String(WIFI_CONNECT_TIMEOUT_SEC);
	sd.logStatus_indent(msg);
	msg = "LOGFILE_PATH_DATA: " + String(LOGFILE_PATH_DATA);
	sd.logStatus_indent(msg);
	msg = "LOGFILE_PATH_STATUS: " + String(LOGFILE_PATH_STATUS);
	sd.logStatus_indent(msg);
	msg = "INSOL_REFERENCE_MAX: " + String(INSOL_REFERENCE_MAX);
	sd.logStatus_indent(msg);
	msg = "DAVIS_SPEED_CAL_FACTOR: " + String(DAVIS_SPEED_CAL_FACTOR);
	sd.logStatus_indent(msg);
	msg = "VANE_OFFSET: " + String(VANE_OFFSET);
	sd.logStatus_indent(msg);
	msg = "WIND_DIRECTION_SPEED_THRESHOLD: " + String(WIND_DIRECTION_SPEED_THRESHOLD);
	sd.logStatus_indent(msg);
	msg = "LOOP_TIME_WARNING_THRESHOLD_MS: " + String(LOOP_TIME_WARNING_THRESHOLD_MS);
	sd.logStatus_indent(msg);
}
