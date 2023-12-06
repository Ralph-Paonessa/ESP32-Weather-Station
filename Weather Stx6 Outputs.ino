// ==========================   Output   =================== //

/// <summary>
/// Prints opening header to serial port when app starts.
/// </summary>
void PrintHeader() {
#if defined(VM_DEBUG)
	Serial.println();
	Serial.println(LINE_SEPARATOR);
	Serial.println(F("ESP32 WEATHER STATION"));
	Serial.print(F("  Speed sampling interval = "));
	Serial.print(BASE_PERIOD_SEC, 2); Serial.println(F(" sec."));
	Serial.println(LINE_SEPARATOR);
#endif
}

///// <summary>
///// Logs summary of devices status.
///// </summary>
//void logDeviceStatus() {
//	String msg = "DEVICE STATUS REPORT:";
//	sd.logStatus(msg, gps.dateTime());
//
//	msg = "WiFi connected: ";
//	msg += bool_OK_Error(WiFi.isConnected());
//	sd.logStatus_indent(msg);
//
//	msg = "SD card: ";
//	msg += bool_OK_Error(_isGood_SDCard);
//	sd.logStatus_indent(msg);
//
//	msg = "LittleFS flash file system: ";
//	msg += bool_OK_Error(_isGood_LITTLEFS);
//	sd.logStatus_indent(msg);
//
//	msg = "GPS module - ";
//	msg += bool_OK_Error(_isGood_GPS);
//	sd.logStatus_indent(msg);
//
//	msg = "Dallas temperature sensor: ";
//	msg += bool_OK_Error(_isGood_Temp);
//	sd.logStatus_indent(msg);
//
//	msg = "Radiation shield fan: ";
//	msg += "???";
//	//msg += bool_OK_Error(_isGood_fan);
//	sd.logStatus_indent(msg);
//
//	msg = "Pressure & RH sensor: ";
//	msg += bool_OK_Error(_isGood_PRH);
//	sd.logStatus_indent(msg);
//
//	msg = "Insolation sensor: ";
//	msg += "???";
//	sd.logStatus_indent(msg);
//
//	msg = "UV sensor: ";
//	msg += bool_OK_Error(_isGood_UV);
//	sd.logStatus_indent(msg);
//
//	msg = "Wind direction sensor: ";
//	msg += "???";
//	sd.logStatus_indent(msg);
//
//	msg = "Wind speed sensor: ";
//	msg += "???";
//	sd.logStatus_indent(msg);
//
//	msg = "Time zone offset from UTC: ";
//	msg += gps.timeZoneOffset();
//	sd.logStatus_indent(msg);
//
//	msg = "Is Daylight Time: : ";
//	msg += bool_true_false(gps.isDaylightTime());
//	sd.logStatus_indent(msg);
//}

///// <summary>
///// Logs state of debug options.
///// </summary>
//void logDebugStatus() {
//#if defined(VM_DEBUG)
//	sd.logStatus("Compiled as DEBUG BUILD");
//#else
//	sd.logStatus("Compiled as RELEASE BUILD");
//	Serial.print("Compiled as RELEASE BUILD\n");
//#endif
//	sd.logStatus(LINE_SEPARATOR);
//	sd.logStatus("DEBUG FLAGS SET:");
//	if (_isDEBUG_BypassGPS) {
//		sd.logStatus_indent("BYPASS GPS");
//	}
//	if (_isDEBUG_BypassWifi) {
//		sd.logStatus_indent("BYPASS WIFI");
//	}
//	if (_isDEBUG_ListLittleFS) {
//		sd.logStatus_indent("LIST LittleFS CONTENTS");
//	}
//	if (_isDEBUG_BypassSDCard) {
//		Serial.println("BYPASS SD CARD");	// Can't log to SD!
//	}
//	if (_isDEBUG_BypassWebServer) {
//		sd.logStatus_indent("BYPASS WEB SERVER");
//	}
//	if (_isDEBUG_run_test_in_setup) {
//		sd.logStatus_indent("RUN TEST CODE IN SETUP");
//	}
//	if (_isDEBUG_run_test_in_loop) {
//		sd.logStatus_indent("RUN TEST CODE IN LOOP");
//	}
//	if (_isDEBUG_addDummyDataLists) {
//		sd.logStatus_indent("ADD DUMMY DATA");
//	}
//	if (_isDEBUG_simulateSensorReadings) {
//		sd.logStatus_indent("USE DUMMY DATA FOR SENSOR READINGS");
//	}
//	if (_isDEBUG_simulateWindReadings) {
//		sd.logStatus_indent("USE DUMMY DATA FOR WIND READINGS");
//	}
//	if (_isDEBUG_AddDelayInLoop) {
//		String msg = String(_LOOP_DELAY_DEBUG_ms);
//		msg += " ms DELAY ADDED IN LOOP";
//		sd.logStatus_indent(msg);
//	}
//	sd.logStatus_indent("End of debug flags");
//	sd.logStatus(LINE_SEPARATOR);
//}
//
///// <summary>
///// Logs space used by LittleFS.
///// </summary>
//void logLittleFsSpaceUsage() {
//	String msg = "LittleFS Total space: ";
//	msg += String(LittleFS.totalBytes() / 1000.);
//	msg += " KB";
//	sd.logStatus_indent(msg);
//	msg = "LittleFS space used: ";
//	msg += String(LittleFS.usedBytes() / 1000.);
//	msg += " KB";
//	sd.logStatus_indent(msg);
//	msg = "LittleFS space available: ";
//	msg += String((LittleFS.totalBytes() - LittleFS.usedBytes()) / 1000.);
//	msg += " KB";
//	sd.logStatus_indent(msg);
//}
//
//
//
///// <summary>
///// LogS the application settings to the STATUS file.
///// </summary>
//void logApp_Settings() {
//	String msg = "APP SETTINGS:";
//	sd.logStatus(msg);
//	msg = "BASE_PERIOD_SEC: " + String(BASE_PERIOD_SEC);
//	msg += " sec";
//	sd.logStatus_indent(msg);
//	msg = "GPS_CYCLES_FOR_SYNC: " + String(GPS_CYCLES_FOR_SYNC);
//	sd.logStatus_indent(msg);
//	msg = "GPS_CYCLES_COUNT_MAX: " + String(GPS_CYCLES_COUNT_MAX);
//	sd.logStatus_indent(msg);
//	msg = "GPS_DELAY_BETWEEN_CYCLES: " + String(GPS_CYCLE_DELAY_SEC);
//	sd.logStatus_indent(msg);
//	msg = "GPS_MAX_ALLOWED_HDOP: " + String(GPS_MAX_ALLOWED_HDOP);
//	sd.logStatus_indent(msg);
//	msg = "UTC_OFFSET_HOURS: " + String(UTC_OFFSET_HOURS);
//	sd.logStatus_indent(msg);
//	msg = "IS_DAYLIGHT_TIME: " + String(IS_DAYLIGHT_TIME);
//	sd.logStatus_indent(msg);
//	msg = "WIFI_CONNECT_TIMEOUT_SEC: " + String(WIFI_CONNECT_TIMEOUT_SEC);
//	sd.logStatus_indent(msg);
//	msg = "LOGFILE_PATH_DATA: " + String(LOGFILE_PATH_DATA);
//	sd.logStatus_indent(msg);
//	msg = "LOGFILE_PATH_STATUS: " + String(LOGFILE_PATH_STATUS);
//	sd.logStatus_indent(msg);
//	msg = "INSOL_REFERENCE_MAX: " + String(INSOL_REFERENCE_MAX);
//	sd.logStatus_indent(msg);
//	msg = "DAVIS_SPEED_CAL_FACTOR: " + String(DAVIS_SPEED_CAL_FACTOR);
//	sd.logStatus_indent(msg);
//	msg = "VANE_OFFSET: " + String(VANE_OFFSET);
//	sd.logStatus_indent(msg);
//	msg = "WIND_DIRECTION_SPEED_THRESHOLD: " + String(WIND_DIRECTION_SPEED_THRESHOLD);
//	sd.logStatus_indent(msg);
//	msg = "LOOP_TIME_WARNING_THRESHOLD_MS: " + String(LOOP_TIME_WARNING_THRESHOLD_MS);
//	sd.logStatus_indent(msg);
//}

///////////////  Strings for SD Card Output  //////////////
/*
  Sensor readings saved to SD card.
  ================================
  1.	DateTime
  2.	Temp F
  3.	P at sea lvl, mb
  4.	P absolute, mb
  5.	RH%
  6.	BME Temp, C
  7.	Solar, %
  8.	UV-A
  9.	UV-B
 10.	UV Index
 11.	IR sky temp, C
 12.	Wind speed, mph
 13.	Avg gust, mph
 14.	Max speed, mph
 15.	Wind vane, deg.
 16.	Wind from
 17.	Fan rpm
  ===================
*/

/// <summary>
/// Returns a string of tab-delimited column 
/// names for logging to SD card.
/// </summary>
/// <returns>String</returns>
String columnNames() {
	String s = "Time";									// (1)
	s += "\tT, F";										// (1)
	s += "\tP, mb, sea lvl\tP, mb, abs";				// (2)
	s += "\tRH %\tBME T, C";							// (2)
	s += "\tSolar, %";									// (1)
	s += "\tUV - A\tUV - B\tUV Index";					// (3)
	s += "\tIR sky temp, C";							// (1)
	s += "\tWind speed, 10-min, mph\tGust, mph";		// (2)
	s += "\tMax speed Instant, mph";					// (1)
	s += "\tWind vane, deg\tWind from";					// (2)
	s += "\tFan rpm";									// (1)
	return s;											// (18 total)
}

/// <summary>
/// Returns current readings for all sensors, 
/// as a delimited string.
/// </summary>
/// <returns>String</returns>
String sensorsDataString_current() {
	// time (1)
	String s = gps.dateTime();
	// temperature (1)
	s += "\t" + String(d_Temp_F.valueLastAdded());
	// pressure (2)
	s += "\t" + String(d_Pres_seaLvl_mb.valueLastAdded());	// mb adjusted to sea level
	s += "\t" + String(d_Pres_mb.valueLastAdded());			// absolute mb (hPa)
	// RH (2)
	s += "\t" + String(d_RH.valueLastAdded());				// %RH
	s += "\t" + String(d_Temp_for_RH_C.valueLastAdded());	// temp recorded by BME280
	// Solar (1)
	s += "\t" + String(d_Insol.valueLastAdded());			// PV solar cell %
	// UV (3)
	if (_isGood_UV) {
		s += "\t" + String(d_UVA.valueLastAdded());
		s += "\t" + String(d_UVB.valueLastAdded());
		s += "\t" + String(d_UVIndex.valueLastAdded());		// Scale 0-10+
	}
	else {
		s += "\tna\tna\tna";
	}
	// IR sky (1)
	s += "\t" + String(d_IRSky_C.valueLastAdded());
	// Wind speed (3)
	s += "\t" + String(windSpeed.avg_10_min());
	s += "\t" + String(windGust.max_10_min().value);
	s += "\tna";	// + String(windSpeed.max_last_10_min);  XXX  ???
	// Wind direction (2)
	if (windSpeed.avg_10_min() > 0.5)
	{
		s += "\t" + String(windDir.angleAvg_now());
		s += "\t" + windDir.directionCardinal();
	}
	else {
		// WindSpeed too low to determine direction.
		s += "\tna";
		s += "\tna";
	}
	// Fan	(1)
	s += "\t" + String(d_fanRPM.valueLastAdded());
	return s;
}

/// <summary>
/// Returns 10-min averages for all 
/// sensors as a delimited string.
/// </summary>
/// <returns>String</returns>
String sensorsDataString_10_min() {
	// time (1)
	String s = gps.dateTime();
	// temperature (1)
	s += "\t" + String(d_Temp_F.avg_10_min());
	// pressure (2)
	s += "\t" + String(d_Pres_seaLvl_mb.avg_10_min());	// mb adjusted to sea level
	s += "\t" + String(d_Pres_mb.avg_10_min());			// absolute mb (hPa)
	// RH (2)
	s += "\t" + String(d_RH.avg_10_min());				// %RH
	s += "\t" + String(d_Temp_for_RH_C.avg_10_min());	// temp recorded by BME280
	// Solar (1)
	s += "\t" + String(d_Insol.avg_10_min());			// PV solar cell mV
	// UV (3)
	if (_isGood_UV) {
		s += "\t" + String(d_UVA.avg_10_min());
		s += "\t" + String(d_UVB.avg_10_min());
		s += "\t" + String(d_UVIndex.avg_10_min());		// Scale 0-10+
	}
	else {
		s += "\tna\tna\tna";
	}
	// IR sky (1)
	s += "\t" + String(d_IRSky_C.avg_10_min());
	// Wind speed (3)
	s += "\t" + String(windSpeed.avg_10_min());
	s += "\t" + String(windGust.max_10_min().value);
	s += "\tmax?";	// + String(windSpeed.max_last_10_min);  XXX  ???
	// Wind direction (2)
	if (windSpeed.avg_10_min() >= WIND_DIRECTION_SPEED_THRESHOLD)
	{
		s += "\t" + String(windDir.avg_10_min());
		s += "\t" + windDir.directionCardinal();
	}
	else {
		// Speed too low to report wind direction.
		s += "\tna";
		s += "\tna";
	}
	// Fan	(1)
	s += "\t" + String(d_fanRPM.avg_10_min());
	return s;
}

/// <summary>
/// Prints tabbed column headings to serial monitor.
/// </summary>
void PrintColumnHeadings() {
//#if defined(VM_DEBUG)
	// Printing column headings to the serial monitor
	// (Tabs are every 8 characters.)
	Serial.print(F("Temperature\t"));
	Serial.print(F("P (sea lvl)\tP (sea lvl)\tP (abs)"));
	Serial.print(F("\t\tHumidity\tTemp (BME)\t"));
	Serial.print(F("Solar\t\t"));
	Serial.print(F("UV - A\tUV - B\tUV Index\t"));
	Serial.print(F("IR\tamb\t"));
	Serial.print(F("Wind avg\tMov Avg\t\tMaximum\t\t"));
	Serial.print(F("From\t"));
	Serial.print(F("Fan rpm"));
	// Advance to new line.
	Serial.println();
//#endif
}

/// <summary>
/// Prints a tabbed line of current 
/// sensor data to serial monitor.
/// </summary>
void PrintSensorOutputs() {
#if defined(VM_DEBUG)
	// Temperature.
	Serial.print(d_Temp_F.valueLastAdded(), 1);
	Serial.print(F("ºF")); Serial.print(F("\t"));
	// Adjust to sea level:
	Serial.print(d_Pres_seaLvl_mb.valueLastAdded(), 1);
	Serial.print(F(" mbar\t"));	// mbar = hPa
	// Absolute pressure
	Serial.print(d_Pres_mb.valueLastAdded(), 1); Serial.print(F(" mb\t"));
	// Relative Humidity
	Serial.print(d_RH.valueLastAdded(), 1); Serial.print(F(" % \t\t"));
	Serial.print(d_Temp_for_RH_C.valueLastAdded()); Serial.print(F("ºC\t\t"));
	// Solar radiation.
	Serial.print(d_Insol.valueLastAdded(), 1); Serial.print(F(" mV\t"));
	// UV sensor.
	// Use the uva, uvb, and index functions to
	// read calibrated UVA and UVB values and a
	// calculated UV index value between 0-11.
	if (_isGood_UV) {
		Serial.print(d_UVA.valueLastAdded()); Serial.print(F("\t"));
		Serial.print(d_UVB.valueLastAdded()); Serial.print(F("\t"));
		Serial.print(d_UVIndex.valueLastAdded()); Serial.print(F("\t\t"));
	}
	else {
		Serial.print("na"); Serial.print(F("\t"));
		Serial.print("na"); Serial.print(F("\t"));
		Serial.print("na"); Serial.print(F("\t\t"));
	}// Remote Infrared.
	Serial.print(d_IRSky_C.valueLastAdded(), 1);
	Serial.print(F("ºC\t"));
	//////Serial.print(windSpeed.speed_last_2_min(), 1);
	//////Serial.print(F(" mph  \t"));

	Serial.print(windSpeed.avg_10_min(), 1);
	Serial.print(F(" mph  \t"));

	Serial.print(windGust.max_10_min().value, 1);
	Serial.print(F(" mph  \t"));

	// Wind direction.
	Serial.print(windDir.angleAvg_now());
	Serial.print(windDir.angleAvg_now()); Serial.print(F("deg\t"));
	Serial.print(windDir.directionCardinal()); Serial.print(F("\t"));
	Serial.print(d_fanRPM.valueLastAdded()); Serial.print(F(" rpm"));
	// Advance to new line.
	Serial.println();
#endif
}
