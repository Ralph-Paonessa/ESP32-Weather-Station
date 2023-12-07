

WindSpeed windSpeed(
	DAVIS_SPEED_CAL_FACTOR,
	true,
	WIND_SPEED_NUMBER_IN_MOVING_AVG,
	WIND_SPEED_OUTLIER_DELTA);	// WindSpeed instance for wind.
SensorData windGust;
WindDirection windDir(VANE_OFFSET);	// WindDirection instance for wind.


// ==========   SENSORS   ========================== //

// ==========   MLX90614 Infrared Temperature   =================== //
// Requires <Adafruit_MLX90614.h>
// default address 0x5A
Adafruit_MLX90614 sensor_IR = Adafruit_MLX90614();	// MLX90614 IR temperature sensor.

// ==========   VEML6075 UV-A/B   ========================= //
// Requires <SparkFun_VEML6075_Arduino_Library.h>
VEML6075 sensor_UV;    // VEML6075 UV sensor.

// ==========   BME280 T/P/RH sensor   =================== //
// Requires <Adafruit_Sensor.h>, <Adafruit_BME280.h>
// Create BME280 instance. I2C address 0x77.
Adafruit_BME280 sensor_PRH; // BME280 temperature sensor.

// ==========   DS18B20 digital temperature sensor   ================== //
// Requires <OneWire.h>
// Requires <DallasTemperature.h>
// Setup a oneWire instance to communicate with any OneWire devices.
OneWire oneWire(ONE_WIRE_PIN);
// Pass our oneWire reference to Dallas Temperature sensor.
DallasTemperature sensor_T(&oneWire);

// 1-Wire routines  ///////////////////////////

/// <summary>
/// Returns count of detected one-wire devices.
/// </summary>
/// <returns></returns>
int countOneWireDevices() {
	// Find all 1-Wire devices on (global) oneWire interface.
	uint8_t address[8];             // holds address returned from search
	int count = 0;                  // count devices found
	// Search for all devices.
	if (oneWire.search(address)) {	// populates address
		count++;
	}
	return count;
}

/// <summary>
/// Returns temperature (F) from DS18B20 sensor.
/// </summary>
/// <returns></returns>
float reading_Temp_F_DS18B20() {
	sensor_T.requestTemperatures();	// Begin DS18B20 sensor read.
	float temp = sensor_T.getTempFByIndex(0);	// Retrieve reading
	return temp;
}

/// <summary>
/// Insolation (solar panel output) in mV.
/// </summary>
/// <returns></returns>
float readInsol_mV() {
	/*
	 Default resolution 12-bit (0-4095).
	 Maximum analog input is 3.2V.
	 Values from 0-100 mV or 3200-3300 mV
	 not distinguishable (NON-LINEAR range).
	 Return mV based on full scale (4095) = 3200mV.
	*/
	return (analogRead(SOLAR_PIN) / 4096.) * 3200.;    // Solar panel output in mV.
}

/// <summary>
/// Starts all sensors.
/// </summary>
void sensors_begin() {
	//  ---------------  BME280 P, RH sensor   ---------------
	// Create BME280 sensor.
	if (!sensor_PRH.begin(0x77)) {
		String msg = "WARNING: BME280 P/RH sensor not found.";
		sd.logStatus(msg, millis());
		_isGood_PRH = false;
	}
	else {
		_isGood_PRH = true;
		String msg = "BME280 P/RH sensor found.";
		sd.logStatus(msg, millis());
	}
	//  ---------------  DS18B20 T sensor   ---------------
	sensor_T.begin();
	// Find DS18B20 temperature.
	if (countOneWireDevices() < 1) {
		_isGood_Temp = false;
		String msg = "WARNING: DS18B20 T sensor not found.";
		sd.logStatus(msg, millis());
	}
	else {
		_isGood_Temp = true;
		String msg = "DS18B20 T sensor found.";
		sd.logStatus(msg, millis());
	}
	//  ---------------  VEML6075 UV sensor   ---------------
	// The VEML6075 begin returns true on success
	// or false on failure to communicate.
	if (sensor_UV.begin() == true) {
		_isGood_UV = true;
		String msg = "VEML6075 UV sensor found.";
		sd.logStatus(msg, millis());
	}
	else {
		String msg = "WARNING: VEML6075 UV sensor not found.";
		sd.logStatus(msg, millis());
	}
	//  ---------------  MLX90614 IR sensor   ---------------
	sensor_IR.begin();
	if (sensor_IR.readAmbientTempC() > 1000) {
		/*
		Missing sensor gives T > 1000.
		Implies IR sensor was not found.
		*/
		_isGood_IR = false;
		String msg = "WARNING: MLX90614 IR sensor not found.";
		sd.logStatus(msg, millis());
	}
	else {
		_isGood_IR = true;
		String msg = "MLX90614 IR sensor found.";
		sd.logStatus(msg, millis());
	}
	//  ---------------  Davis wind sensors   ---------------
	windDir.begin();	// Create WindDirection.
	String msg = "[ No connection test implemented for Davis anemometer. ]";
	sd.logStatus(msg, millis());
	_isGood_WindDir = true;      // How can this be tested?? XXX
	_isGood_WindSpeed = true;

	//  ---------------  Create LittleFS   ---------------
	if (!LittleFS.begin()) {
		String msg = "ERROR: LittleFS didn't mount.";
		sd.logStatus(msg, millis());
	}
	else {
		_isGood_LITTLEFS = true;
		String msg = "LittleFS mounted.";
		sd.logStatus(msg, millis());
	}
	// Log used and available space.
	logLittleFsSpaceUsage();

	if (_isDEBUG_ListLittleFS) {
		dirList(LittleFS, "/", 5);
	}

	sd.logStatus("Device initialization complete.", millis());
	logDeviceStatus();
}

/******   WIND READINGS    ******/

/// <summry>
/// Reads and saves wind speed, gusts, and direction.
/// </summary>
void readWind() {
	if (_isDEBUG_simulateWindReadings) {
		readWind_Simulate();
		return;
	}
	// Read wind speed.
	float speed = windSpeed.speedInstant(_anem_Rotations, BASE_PERIOD_SEC);
	dataPoint dpSpeed(now(), speed);
	windSpeed.addReading(dataPoint(dpSpeed));

	// Record any gusts.
	float avg = windSpeed.avg_now();
	dataPoint dpGust = windSpeed.gust(dpSpeed, avg);
	windGust.addReading(dpGust);



	// Read wind direction.
	float windAngle = windAngleReading();
	windDir.addReading(now(), windAngle, speed);	// weighted by speed

	// Reset hardware interrupt count.
	portENTER_CRITICAL_ISR(&hardwareMux_anem);
	_anem_Rotations = 0;	// Reset anemometer count.
	portEXIT_CRITICAL_ISR(&hardwareMux_anem);
}

/// <summary>
/// Returns number of rotations that produce a given speed.
/// </summary>
/// <param name="speed">Speed, mph</param>
/// <returns>Number of rotations.</returns>
float rotsFromSpeed(float speed) {
	return speed * BASE_PERIOD_SEC / DAVIS_SPEED_CAL_FACTOR;
}

// =======   Wind speed measurement and averaging   ================== //

/// <summary>
/// Reads and returns raw wind angle from anemometer 
/// analog pin as integer value from 0 - 360 deg.
/// </summary>
/// <returns>Integer wind angle.</returns>
int windAngleReading() {
	int vaneValue = analogRead(WIND_VANE_PIN);
	return map(vaneValue, 0, 4095, 0, 359);	// Map to 0-359 deg.	
}

/// <summary>
/// Adds simulated wind speed readings.
/// </summary>
void readWind_Simulate() {
#if defined(VM_DEBUG)
	float sim_speed_start = 3;
	float sim_speed_spike = 15;
	float sim_speed_incr = 0.025;
	unsigned int rots = dummy_anemCount.linear
	(
		rotsFromSpeed(sim_speed_start),
		rotsFromSpeed(sim_speed_incr),
		rotsFromSpeed(sim_speed_spike),
		50,
		6
	);
	float speed = windSpeed.speedInstant(rots, BASE_PERIOD_SEC);
	dataPoint dpSpeed(now(), speed);
	windSpeed.addReading(dpSpeed);

	// XXX  DOES THIS WORK FOR SIMULATION?!?!  XXX
	// Record any gusts. Use MOVING AVG of wind speed.
	float avg_moving = windSpeed.avgMoving();
	dataPoint dpGust = windSpeed.gust(dpSpeed, avg_moving);
	windGust.addReading(dpGust);

	// Read wind direction.
	float windAngle = dummy_windDir.sawtooth(90, 1, 360);
	windDir.addReading(now(), windAngle, dpSpeed.value);	// weighted by speed
#endif
}


/// <summary>
/// Reads and saves fan speed.
/// </summary>
void readFan() {
	// Get fan speed.
	d_fanRPM.addReading(dataPoint(now(), fanRPM(_fanHalfRots, BASE_PERIOD_SEC)));
	// Reset hardware interrupt count.
	portENTER_CRITICAL(&hardwareMux_fan);
	_fanHalfRots = 0;		// Reset fan rotations.		
	portEXIT_CRITICAL(&hardwareMux_fan);
}

/// <summary>
/// Reads and saves data from sensors.
/// </summary>
void readSensors() {
	unsigned long timeStart = millis();
	if (_isDEBUG_simulateSensorReadings) {
		// Simulate sensor readings.
		readSensors_Simulate();
		return;
	}
	dataPoint dp;	// holds successive readings
	// Temperature.
	dp = dataPoint(now(), reading_Temp_F_DS18B20());
	d_Temp_F.addReading(dp);
	// UV readings.
	dp = dataPoint(now(), sensor_UV.uva());
	d_UVA.addReading(dp);
	dp = dataPoint(now(), sensor_UV.uvb());
	d_UVB.addReading(dp);
	dp = dataPoint(now(), sensor_UV.index());
	d_UVIndex.addReading(dp);
	// P, RH
	dp = dataPoint(now(), sensor_PRH.readHumidity());
	d_RH.addReading(dp);
	dp = dataPoint(now(), sensor_PRH.readPressure() / 100);
	d_Pres_mb.addReading(dp);			// Raw pressure in mb (hectopascals)
	dp = dataPoint(now(), sensor_PRH.readTemperature());
	d_Temp_for_RH_C.addReading(dp);		// Temp (C) of P, RH sensor.
	// P adjusted to sea level.
	float psl = pressureAtSeaLevel(
		d_Pres_mb.valueLastAdded(),
		gps.data.altitude(),
		d_Temp_for_RH_C.valueLastAdded());
	dp = dataPoint(now(), psl);
	d_Pres_seaLvl_mb.addReading(dp);
	// IR sky
	dp = dataPoint(now(), sensor_IR.readObjectTempC());
	d_IRSky_C.addReading(dp);
	// Insolation/
	float insol_norm = insol_norm_pct(readInsol_mV(), INSOL_REFERENCE_MAX);
	dp = dataPoint(now(), insol_norm);
	d_Insol.addReading(dp);	// % Insolation

	unsigned int timeEnd = millis() - timeStart;
}

/// <summary>
/// Adds simulated values to sensor readings 
/// (doesn't include wind readings).
/// </summary>
void readSensors_Simulate() {
#if defined(VM_DEBUG)
	dataPoint dp;	// holds reading
	// Temperature.
	dp = dataPoint(now(), dummy_Temp_F.sawtooth(10, 0.02, 20));
	d_Temp_F.addReading(dp);
	// UV readings.
	/*dp = dataPoint(now(), dummy_UVA.linear(3, 0.1));
	d_UVA.addReading(dp);
	dp = dataPoint(now(), dummy_UVB.linear(3, 0.1));
	d_UVB.addReading(dp);*/
	dp = dataPoint(now(), dummy_UVIndex.sawtooth(0, 0.05, 10));
	d_UVIndex.addReading(dp);
	// P, RH
	dp = dataPoint(now(), dummy_RH.sawtooth(0, 0.05, 50));
	d_RH.addReading(dp);
	dp = dataPoint(now(), dummy_Pres_mb.linear(3, 0.1) / 100);
	d_Pres_mb.addReading(dp);			// Raw pressure in mb (hectopascals)
	//dp = dataPoint(now(), dummy_Temp_for_RH_C.linear(10, 0.02));
	//d_Temp_for_RH_C.addReading(dp);		// Temp (C) of P, RH sensor.
	// P adjusted to sea level.
	float psl = pressureAtSeaLevel(
		dummy_Pres_seaLvl_mb.linear(950, 0.01),
		gps.data.altitude(),
		25);
	dp = dataPoint(now(), psl);
	d_Pres_seaLvl_mb.addReading(dp);
	// IR sky
	dp = dataPoint(now(), dummy_IRSky_C.sawtooth(10, 0.02, 20));	// .sawtooth(0, 0.02, 10));
	d_IRSky_C.addReading(dp);
	// Insolation/
	float insol_norm = insol_norm_pct(dummy_Insol.linear(0, 0.01), INSOL_REFERENCE_MAX);
	dp = dataPoint(now(), insol_norm);
	d_Insol.addReading(dp);	// % Insolation
#endif
}

/// <summary>
/// Saves last read time to LittleFS.
/// </summary>
/// <param name="time">Time to save.</param>
void saveLastReadTime_toFile(unsigned long time) {
	// Save in LittleFS
	//if (_isDataInFileSys) {
	fileWrite(LittleFS, SENSOR_DATA_TIME_FILE_PATH.c_str(), String(time).c_str());
	///}
}

/// <summary>
/// Gets last reading time from LittleFS.
/// </summary>
/// <returns>Saved time of last reading.</returns>
unsigned long lastReadingTime_fromFile()
{
	// Read from LittleFS
	//if (_isDataInFileSys) {
	return fileReadString(LittleFS, SENSOR_DATA_TIME_FILE_PATH.c_str()).toInt();
	//}
	//else {
	//	return 0;
}

/// <summary>
/// Saves last read t to LittleFS.
/// </summary>
/// <param name="t">Time to save.</param>
void saveLastReadTime_toFile(unsigned long t) {
	// Save in LittleFS
	//if (_isDatafile) {
	fileWrite(LittleFS, SENSOR_DATA_TIME_FILE_PATH.c_str(), String(t).c_str());
	///}
}

/// <summary>
/// Gets last reading t from LittleFS.
/// </summary>
/// <returns>Saved t of last reading.</returns>
unsigned long lastReadingTime_fromFile()
{
	// Read from LittleFS
	//if (_isDatafile) {
	return fileRead(LittleFS, SENSOR_DATA_TIME_FILE_PATH.c_str()).toInt();
	//}
	//else {
	//	return 0;
}

/// <summary>
/// Saves 10-min averages of all sensor data 
/// to lists.
/// </summary>
void processReadings_10_min() {
	windSpeed.process_data_10_min();
	windGust.process_data_10_min();
	windDir.process_data_10_min();
	d_Temp_F.process_data_10_min();
	d_Pres_mb.process_data_10_min();		// Just save avg_10?
	d_Pres_seaLvl_mb.process_data_10_min();
	d_Temp_for_RH_C.process_data_10_min();	// Just save avg_10?
	d_RH.process_data_10_min();
	d_UVA.process_data_10_min();
	d_UVB.process_data_10_min();
	d_UVIndex.process_data_10_min();
	d_Insol.process_data_10_min();
	d_IRSky_C.process_data_10_min();

	// Save last 10-min reading time to LittleFS. Used 
	// to check whether to recover data at reboot.
	saveLastReadTime_toFile(now());

}

/// <summary>
/// Saves 60-min averages to lists.
/// </summary>
void processReadings_60_min() {
	windSpeed.process_data_60_min();
	windGust.process_data_60_min();
	windDir.process_data_60_min();
	d_Temp_F.process_data_60_min();
	d_Pres_seaLvl_mb.process_data_60_min();
	d_RH.process_data_60_min();
	d_UVA.process_data_60_min();
	d_UVB.process_data_60_min();
	d_UVIndex.process_data_60_min();
	d_Insol.process_data_60_min();
	d_IRSky_C.process_data_60_min();
}
/// <summary>
/// Saves all readings minima and maxima 
/// for the prior day.
/// </summary>
void processReadings_day() {
	windSpeed.process_data_day();
	windDir.process_data_day();
	d_Temp_F.process_data_day();
	d_Pres_seaLvl_mb.process_data_day();
	d_RH.process_data_day();
	d_UVA.process_data_day();
	d_UVB.process_data_day();
	d_UVIndex.process_data_day();
	d_Insol.process_data_day();
	d_IRSky_C.process_data_day();
}

/*******  DUMMY DATA   ********/

/// <summary>
/// Adds dummy data to SensorData instance lists.
/// </summary>
void addDummyData() {
#if defined(VM_DEBUG)
	Serial.printf("\naddDummyData now() = %li\n", now());

	// 10-min
	d_Temp_F.addDummy_data_10_min(65, -0.75, 24, 1765412100);
	d_Pres_mb.addDummy_data_10_min(991, 1, 24, 1765412100);
	d_Pres_seaLvl_mb.addDummy_data_10_min(991, 1, 24, 1765412100);
	d_RH.addDummy_data_10_min(20, .5, 24, 1765412100);
	d_IRSky_C.addDummy_data_10_min(-25, 0.5, 24, 1765412100);
	windSpeed.addDummy_data_10_min(15, 0.5, 24, 1765412100);
	windGust.addDummy_data_10_min(25, 2, 24, 1765412100);
	windDir.addDummy_data_10_min(270, 5, 24, 1765412100);
	d_Insol.addDummy_data_10_min(2700, 25, 24, 1765412100);
	d_UVIndex.addDummy_data_10_min(0, 0.5, 24, 1765412100);

	processReadings_10_min();

	// 60-min
	d_Temp_F.addDummy_data_60_min(65, 0.1, 24, 1765412100);
	d_Pres_mb.addDummy_data_60_min(989, 1.5, 24, 1765412100);
	d_Pres_seaLvl_mb.addDummy_data_60_min(991, 2, 24, 1765412100);
	d_RH.addDummy_data_60_min(20, .5, 24, 1765412100);
	d_IRSky_C.addDummy_data_60_min(-25, 0.5, 24, 1765412100);
	windSpeed.addDummy_data_60_min(15, 0.5, 24, 1765412100);
	windGust.addDummy_data_60_min(25, 2, 24, 1765412100);
	windDir.addDummy_data_60_min(270, 5, 24, 1765412100);
	d_Insol.addDummy_data_60_min(2700, 25, 24, 1765412100);
	d_UVIndex.addDummy_data_60_min(0, 0.5, 24, 1765412100);

	processReadings_60_min();

	// daily maxima
	d_Temp_F.addDummy_data_dayMax(65, 1, 10, 1765412100);
	d_Pres_mb.addDummy_data_dayMax(989, 1.5, 24, 1765412100);
	d_Pres_seaLvl_mb.addDummy_data_dayMax(991, 2, 24, 1765412100);
	d_RH.addDummy_data_dayMax(20, .5, 24, 1765412100);
	d_IRSky_C.addDummy_data_dayMax(-25, 0.5, 24, 1765412100);
	windSpeed.addDummy_data_dayMax(15, 0.5, 24, 1765412100);
	windGust.addDummy_data_dayMax(25, 2, 24, 1765412100);
	windDir.addDummy_data_dayMax(270, 5, 24, 1765412100);
	d_Insol.addDummy_data_dayMax(2700, 25, 24, 1765412100);
	d_UVIndex.addDummy_data_dayMax(0, 0.5, 24, 1765412100);

	// daily minima
	d_Temp_F.addDummy_data_dayMin(45, -1, 10, 1765412400);
	d_Pres_mb.addDummy_data_dayMin(989, 1.5, 24, 1765412100);
	d_Pres_seaLvl_mb.addDummy_data_dayMin(991, 2, 24, 1765412100);
	d_RH.addDummy_data_dayMin(20, .5, 24, 1765412100);
	d_IRSky_C.addDummy_data_dayMin(-25, 0.5, 24, 1765412100);
	windSpeed.addDummy_data_dayMin(15, 0.5, 24, 1765412100);
	windGust.addDummy_data_dayMin(25, 2, 24, 1765412100);
	windDir.addDummy_data_dayMin(270, 5, 24, 1765412100);
	d_Insol.addDummy_data_dayMin(2700, 25, 24, 1765412100);
	d_UVIndex.addDummy_data_dayMin(0, 0.5, 24, 1765412100);

	processReadings_day();

#endif
}

/// <summary>
/// Adds labels and units to SensorData instances.
/// </summary>
void sensors_AddLabels()
{

	windSpeed.addLabels("Wind Speed", "wind", "mph");
	windDir.addLabels("Wind direction", "windDir", "", "&deg;");
	windGust.addLabels("Wind Gust", "gust", "mph");
	d_Temp_F.addLabels("Temperature", "temp", "F", "&deg;F");
	d_Pres_mb.addLabels("Pressure (abs)", "presAbs", "mb");
	d_Pres_seaLvl_mb.addLabels("Pressure (SL)", "presSeaLvl", "mb");
	d_Temp_for_RH_C.addLabels("Temp for RH", "tForRH", "C", "&degC;");
	d_RH.addLabels("Rel. Humidity", "RH", "%", "&percnt;");
	d_IRSky_C.addLabels("Sky Temperature", "skyTemp", "C", "&degC;");
	d_UVA.addLabels("UV A Radiation", "uvA", "");
	d_UVB.addLabels("UV B Radiation", "uvB", "");
	d_UVIndex.addLabels("UV Index", "uvIndex", "");
	d_Insol.addLabels("Insolation", "sun", "%", "&percnt;");
	d_fanRPM.addLabels("Aspirator Fan speedInstant", "fanSpeed", "rpm");
}

/// <summary>
/// Creates data files for selected SensorData instances 
/// that save chart data on the file system.
/// </summary>
void sensors_createFiles()
{
	windSpeed.createFiles();
	windDir.createFiles();
	windGust.createFiles();
	d_Temp_F.createFiles();
	//d_Pres_mb.createFiles();
	d_Pres_seaLvl_mb.createFiles();
	//d_Temp_for_RH_C.createFiles(); 
	d_RH.createFiles();
	d_IRSky_C.createFiles();
	//d_UVA.createFiles();         
	//d_UVB.createFiles();         
	d_UVIndex.createFiles();
	d_Insol.createFiles();
	//d_fanRPM.createFiles();      
}