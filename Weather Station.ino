/*****************************************
Weather-Star 12.0
Custom weather station control software.

Ralph Paonessa
August 10, 2022
Rev. October 7, 2023
******************************************/

// ========  ESP32 Libraries  ================  

// ESP Async Web Server
#include <AsyncEventSource.h>
#include <AsyncJson.h>
#include <AsyncWebSocket.h>
#include <AsyncWebSynchronization.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <StringArray.h>
#include <WebAuthentication.h>
#include <WebHandlerImpl.h>
#include <WebResponseImpl.h>
#include <WebResponseImpl.h>
#include <AsyncTCP.h>

// Sensors
#include <SparkFun_VEML6075_Arduino_Library.h>

// MLX90614 IR sensor
#include <Adafruit_MLX90614.h>

// DS18B20 digital temperature sensor
#include <DallasTemperature.h>
#include <OneWire.h>

#include <Adafruit_BME280.h>

// File system
#include <LittleFS.h>

// WiFi
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

// ========  Custom Libraries  ================  

#include "App_settings.h"
using namespace App_Settings;
#include "PinAssignments.h"
#include "Utilities.h"
using namespace Utilities;
#include "FileOperations.h"
using namespace FileOperations;
#include "GPSModule.h"
#include "SDCard.h"
#include "dataPoint.h"
#include "SensorData.h"
#include "WindSpeed2.h"
#include "WindDirection.h"
#include "DebugFlags.h"


#if defined(VM_DEBUG)
#include "Testing.h"			// DEBUG AND TESTING
#include "SensorSimulate.h"
Testing test;					// class for test routings
#endif


//char _char_global_buffer[2048] = {  };			// Globally-defined character array buffer.

/*
SensorData instances to average readings.
Wind speed handled by WindSpeed.
Wind direction handled by WindDirection.
*/

SensorData d_Temp_F;				// Temperature readings.
SensorData d_Pres_mb;				// Pressure readings.
SensorData d_Pres_seaLvl_mb;		// Pressure readings.
SensorData d_Temp_for_RH_C(false);	// Sensor temperature for pressure readings.
SensorData d_RH;					// Rel. humidity readings.
SensorData d_UVA(false);			// UVA readings.
SensorData d_UVB(false);			// UVB readings.
SensorData d_UVIndex;				// UV Index readings.
SensorData d_Insol(true, true);		// Insolation readings (no minima).
SensorData d_IRSky_C;				// IR sky temperature readings.
SensorData d_fanRPM(false);			// Fan RPM readings.

#if defined(VM_DEBUG)
SensorSimulate dummy_Temp_F;			// Temperature readings.
SensorSimulate dummy_Pres_mb;			// Pressure readings.
SensorSimulate dummy_Pres_seaLvl_mb;	// Pressure readings.
SensorSimulate dummy_Temp_for_RH_C;		// Sensor temperature for pressure readings.
SensorSimulate dummy_RH;				// Rel. humidity readings.
SensorSimulate dummy_UVA;				// UVA readings.
SensorSimulate dummy_UVB;				// UVB readings.
SensorSimulate dummy_UVIndex;			// UV Index readings.
SensorSimulate dummy_Insol;				// Insolaton readings.
SensorSimulate dummy_IRSky_C;			// IR sky temperature readings.
SensorSimulate dummy_fanRPM;			// Fan RPM readings.

SensorSimulate dummy_anemCount;			// Anemometer rot count.
SensorSimulate dummy_windDir;			// Anemometer wind direction.
#endif

// %%%%%%%%%%   STATUS FLAGS FOR DEVICES   %%%%%%%%%%%%%%%%
bool _isGood_Temp = false;
bool _isGood_PRH = false;
bool _isGood_UV = false;
bool _isGood_IR = false;
bool _isGood_WindDir = false;
bool _isGood_WindSpeed = false;
//bool _isGood_GPS = false;
bool _isGood_PMS = false;
//bool _isGood_SDCard = false;
bool _isGood_Solar = false;
bool _isGood_LITTLEFS = false;
bool _isGood_fan = false;
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

unsigned long _timeStart_Loop = 0;			//monitor loop timing

volatile int _countInterrupts_base = 0;		// Timer interrupts for sensor reads.
volatile long _countInterrupts_10_min = 0;	// Timer interrupts for sensor for 10-min averages.
volatile long _countInterrupts_60_min = 0;	// Timer interrupts for sensor for 60-min averages.

// ==========   SD card module   ==================== //
SDCard sd;		// SDCard instance that exposes SD card routines. 

bool _isGood_SDCard = false;
bool _isGood_GPS = false;

// ==========   Async Web Server   ================== //
AsyncWebServer server(80);	// Async web server instance on port 80.

// ==========   u-blox NEO-6M GPS   ========================== //
// GPS module instance. 
GPSModule gps;

int _oldMonth = 0;		// Month of previous day.
int _oldDay = 0;		// Day of previous day.
int _oldYear = 0;		// Year of previous day.

// ==========   PWM Fan for Radiation Shield  ======================== //

volatile unsigned long _fanHalfRots = 0;// count fan half-rotation (2 counts/cycle)

// HARDWARE INTERRUPT for fan tachometer switch that signals half-rotation
portMUX_TYPE hardwareMux_fan = portMUX_INITIALIZER_UNLOCKED;

// Function called by hardware interrupt when fan 
// tachometer switch closes on every half-rotation.
void IRAM_ATTR ISR_onFanHalfRotation() {
	portENTER_CRITICAL_ISR(&hardwareMux_fan);
	_fanHalfRots++;
	portEXIT_CRITICAL_ISR(&hardwareMux_fan);
}

// ==========   Davis anemometer - wind speed  ==================== //
/*
 Wind speed is determined by counting anemometer
 rotations over a brief base measurement period.
 Each rotation triggers a hardware interrupt that
 increments _anem_Rotations by 1.
*/
volatile unsigned int _anem_Rotations = 0;		// Count of anemometer rotations
unsigned long _lastDebounceTime = 0;			// Last millis when output pin was toggled
const unsigned int DEBOUNCE_TIMEOUT = 15;		// Debounce timeout (millisec)

// TIMER INTERRUPT to count anemometer and fan rotations.
hw_timer_t* timer_base = NULL;
portMUX_TYPE timerMux_base = portMUX_INITIALIZER_UNLOCKED;

/// <summary>
/// Timer interrupt service routine to increment interrupt counts.
/// </summary>
void IRAM_ATTR ISR_onTimer_count() {
	portENTER_CRITICAL_ISR(&timerMux_base);
	_countInterrupts_base++;
	_countInterrupts_10_min++;
	_countInterrupts_60_min++;
	portEXIT_CRITICAL_ISR(&timerMux_base);
}

// HARDWARE INTERRUPT that signals one anemometer rotation.
portMUX_TYPE hardwareMux_anem = portMUX_INITIALIZER_UNLOCKED;

// Function called by hardware interrupt when anemometer sensor switch closes.
// It increments the rotation count by 1, after switch debouncing.
void IRAM_ATTR ISR_onRotation_anem() {
	// Ignore any additional activations within the debounce time.
	if ((millis() - _lastDebounceTime) > DEBOUNCE_TIMEOUT) {
		portENTER_CRITICAL_ISR(&hardwareMux_anem);
		_anem_Rotations++;
		portEXIT_CRITICAL_ISR(&hardwareMux_anem);
		_lastDebounceTime = millis();
	}
}
// ========   END Davis Anemometer 6410  =================  //

/// <summary>
/// Check for too many (i.e., unhandled) timer interrupts 
/// before reading and resetting rotation counts (for 
/// anemometer, fan).
/// </summary>
void catchUnhandledBaseTimerInterrupts() {
	// If base timer interrupt count exceeds 1, 
	// the interrupt was unhandled. Can occur when
	// loop processing time exceeds BASE_PERIOD_SEC.
	if (_countInterrupts_base > 1) {	// Should be only 0 or 1.
		String msg = "WARNING: Base timer interrupt count was ";
		msg += String(_countInterrupts_base);
		msg += " indicating unhandled timer interrupts. Reset to 0.";
		sd.logStatus(msg, gps.dateTime());
		portENTER_CRITICAL_ISR(&timerMux_base);
		_countInterrupts_base = 0;	// Reset base timer interrupt.
		portEXIT_CRITICAL_ISR(&timerMux_base);
	}
}

/// <summary>
/// Resets timer interrupt counters. Use when excessive time 
/// in the main loop (such as WiFi reconnection after loss) 
/// causes counts in interrupts to be unhandled.
/// </summary>
void resetTimerInterruptCounts() {
	String msg = "Skip read cycle where _countInterrupts_base was ";
	msg += String(_countInterrupts_base);
	sd.logStatus(msg, millis());

	portENTER_CRITICAL_ISR(&timerMux_base);
	_countInterrupts_base--;
	portEXIT_CRITICAL_ISR(&timerMux_base);

	portENTER_CRITICAL_ISR(&hardwareMux_fan);
	_fanHalfRots = 0;		// Reset fan count.
	portEXIT_CRITICAL_ISR(&hardwareMux_fan);

	portENTER_CRITICAL_ISR(&hardwareMux_anem);
	_anem_Rotations = 0;	// Reset anemometer count.
	portEXIT_CRITICAL_ISR(&hardwareMux_anem);
}

/// <summary>
/// Recover recent sensor readings from LittleFS.
/// </summary>
void recover_data() {
	unsigned long lastTime = lastReadingTime_fromFile();
	// 10-min lists
	if (d_Temp_F.isDataInFileSys()
		&& (now() - lastTime) > DATA_RECOVERY_10_MIN_CUTOFF)
	{
		sd.logStatus("Recovered 10-min data.", millis());

		d_Temp_F.recover_data_10_min_fromFile();
	}

	// 60-min lists
	if (d_Temp_F.isDataInFileSys()
		&& (now() - lastTime) > DATA_RECOVERY_60_MIN_CUTOFF)
	{
		sd.logStatus("Recovered 60-min data.", millis());

		d_Temp_F.recover_data_60_min_fromFile();
	}

	// day lists
	if (d_Temp_F.isDataInFileSys()
		&& (now() - lastTime) > DATA_RECOVERY_DAY_CUTOFF)
	{
		d_Temp_F.recover_data_day_max_min_fromFile();
	}
}

/****************************************************************************/
/******************************      SETUP      *****************************/
/****************************************************************************/
void setup() {

	Serial.begin(115200);

	// Print status message now because SD is not yet online.
	String msg = "\n\n\n" + LINE_SEPARATOR_MAJOR + "\n";
	msg += String(millis() / 1000.);
	msg += "s ENTERING SETUP \nSD card not yet online.\n";
	msg += LINE_SEPARATOR_MAJOR + "\n\n";
	Serial.print(msg);

	//  ==========  CREATE SD CARD   ========== //
	// (Do this first - need SD card for logging.)
	_isGood_SDCard = sd.create(SPI_CS_PIN, _isDEBUG_BypassSDCard);
	// Begin status log entries to SD card.
	sd.logStatus();	// Empty line
	sd.logStatus(LINE_SEPARATOR_MAJOR);
	sd.logStatus("SETUP continues after SD card initialization.", gps.dateTime());
	sd.fileCreateOrExists(LOGFILE_PATH_DATA);
	sd.fileCreateOrExists(LOGFILE_PATH_STATUS);

	// Log the settings to the status file.
	logDebugStatus();
	logApp_Settings();

	//  ==========  CREATE WIFI NETWORK   ========== //	
	wifiSetupAndConnect();

	//  ==========  CREATE ASYNC WEB SERVER   ========== //	
	serverRouteHandler();	// Define routes for server requests.
	sd.logStatus("Async web server routes defined.", millis());
	server.begin();			// Start async web server.
	sd.logStatus("Async web server beginning.", millis());


	// ==========   CREATE GPS AND SYNC TO GET TIME   ========== //
	// XXX  Need code to alter power of GPS!!!  XXX

	/* 	Format for setting s serial port:
		SerialObject.begin(baud-rate, protocol, RX pin, TX pin);
		(Wrong baud rate gives serial garbage.)
	*/
	// Connect to GPS

#define SERIAL_CONFIGURATION SERIAL_8N1	// data, parity, and stop bits
	//const int GPS_BAUD_RATE = 9600;     // Beitian = 9600; Brian's = 38400; NEO-6M 9600
	gps.begin(GPS_BAUD_RATE, SERIAL_CONFIGURATION, RX2_PIN, TX2_PIN);
	sd.logStatus("Connecting to GPS.", millis());
	// Get time and location from GPS.
	// This code is BLOCKING until gps syncs.
	bool isGpsSuccess = false;
	if (!gps.isSynced())
	{
		isGpsSuccess = gps.syncToGPS(sd, _isDEBUG_BypassGPS);
		_isGood_GPS = true;
	}
	if (!isGpsSuccess)
	{
		String msg = "ERROR: GPS did not sync after ";
		msg += String(GPS_CYCLES_COUNT_MAX) + " cycles.";
		sd.logStatus(msg, millis());
	}

	// ==========  CREATE SENSORS  ========== //

	sensors_AddLabels();	// Add labels and units to the SensorData instances.
	sensors_begin();
	sensors_createFiles();

	// Date info to determine when new day begins.
	_oldDay = day();
	_oldMonth = month();
	_oldYear = year();

//#if defined(VM_DEBUG)
	////////  TESTING   ////////
	if (_isDEBUG_addDummyDataLists) {
		addDummyData();
		saveLastReadTime_toFile(now());
		Serial.println();
		Serial.println("XXX  saveLastReadTime_toFile(now())  XXX");
		Serial.println();
	}
	if (_isDEBUG_run_test_in_setup) {
		test.testCodeForSetup3(true);
	}
//#endif

	sd.logData(columnNames());	// Write column names to data log.
	sd.logStatus_indent("DATA COLUMNS:\t" + columnNames());	// Write column names to status log.

	/// ==========  CONFIGURE FAN PWM  ========== //
	const int FAN_PWM_FREQUENCY = 25000;	// 25kHz
	const int FAN_PWM_CHANNEL = 0;
	const int FAN_PWM_RESOLUTION = 8;
	volatile unsigned long _fanHalfRots = 0;// count fan half-rotation (2 counts/cycle)
	ledcSetup(FAN_PWM_CHANNEL, FAN_PWM_FREQUENCY, FAN_PWM_RESOLUTION);
	// Attach the channel to the pin for PWM output.
	ledcAttachPin(FAN_PWM_PIN, FAN_PWM_CHANNEL);
	// Set fan speed using PWM.
	ledcWrite(FAN_PWM_CHANNEL, FAN_DUTY_PERCENT / 100. * 256);

	// Create interrupts after everything else has completed.

	// ==========  CREATE ANEMOMETER HARDWARE INTERRUPT  ========== //
	// Anemometer hardware interrupt for detecting rotation.
	pinMode(WIND_SPEED_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(WIND_SPEED_PIN),
		ISR_onRotation_anem,
		FALLING);
	// Fan hardware interrupt to detect half-rotation.
	pinMode(FAN_SPEED_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(FAN_SPEED_PIN), ISR_onFanHalfRotation, FALLING);

	// ==========  CREATE TIMER INTERRUPT  ========== //
	/*
	 Timer interrupt fires every BASE_PERIOD_SEC to
	 trigger counts of anemometer and fan rotations.
	 This is ALSO when we record all the other sensor readings.
	 ONLY WIND SPEED NEEDS TO BE RECORDED THIS FREQUENTLY.
	 [NOTE: Consider spacing out the other sensor readings.]
	*/
	timer_base = timerBegin(0, 80, true);
	timerAttachInterrupt(timer_base, &ISR_onTimer_count, true);
	int duration_count = BASE_PERIOD_SEC * MICROSEC_PER_SECOND;	// Timer duration (microsec).
	timerAlarmWrite(timer_base, duration_count, true);		// Trigger every BASE_PERIOD_SEC.
	timerAlarmEnable(timer_base);

	msg = "CURRENT LOCAL TIME is " + gps.dateTime();
	(IS_DAYLIGHT_TIME) ? msg = " Daylight time." : msg = " Standard time.";
	sd.logStatus(msg);
	sd.logStatus("SETUP END " + gps.dateTime(), millis());
}
/****************************************************************************/
/************************        END SETUP       ****************************/
/****************************************************************************/

/****************************************************************************/
/***************************       LOOP      ********************************/
/****************************************************************************/
void loop() {
	_timeStart_Loop = millis();	// To monitor loop execution time.

	// Check for timer interrupts that were 
	// not handled during code delays.
	catchUnhandledBaseTimerInterrupts();

	/************************************************
		Read sensors and process data at intervals
		determined from timer interrupt.
	*************************************************/

	//  ====================================================
	//   BASE_PERIOD_SEC. Every timer interrupt.
	if (_countInterrupts_base == 1) {
		// Read sensors and process data.
		readWind();
		readFan();
		// Read data for other sensors.
		readSensors();
		portENTER_CRITICAL_ISR(&timerMux_base);
		_countInterrupts_base--;	// Base timer interrupt handled.
		portEXIT_CRITICAL_ISR(&timerMux_base);
	}

	//   ====================================================
	//    10-MIN INTERVAL.
	if (_countInterrupts_10_min >= BASE_PERIODS_IN_10_MIN) {
		// Get 10-min avgs.
		processReadings_10_min();
		sd.logData(sensorsDataString_10_min());	// Save readings to SD card.
		sd.logStatus("Logged 10-min avgs.", gps.dateTime());
		// Check for unhandled interrupts.
		if (_countInterrupts_10_min > BASE_PERIODS_IN_10_MIN)
		{
			String msg = "WARNING: 10-min interrupt count exceeded threshold by ";
			msg += String(_countInterrupts_base - BASE_PERIODS_IN_10_MIN);
			msg += " indicating unhandled 10-min timer interrupt.";
			sd.logStatus(msg, gps.dateTime());
		}
		portENTER_CRITICAL_ISR(&timerMux_base);
		_countInterrupts_10_min = 0;	// Interrupt handled.
		portEXIT_CRITICAL_ISR(&timerMux_base);
	}

	//   ====================================================
	//    60-MIN INTERVAL
	if (_countInterrupts_60_min >= BASE_PERIODS_IN_60_MIN) {
		processReadings_60_min();
		sd.logData(sensorsDataString_10_min());	// Save readings to SD card.
		sd.logStatus("Logged 60-min avgs.", gps.dateTime());
		// Check for unhandled.
		if (_countInterrupts_60_min > BASE_PERIODS_IN_60_MIN) {
			String msg = "WARNING: 10-min interrupt count exceeded threshold by ";
			msg += String(_countInterrupts_60_min - BASE_PERIODS_IN_60_MIN);
			msg += " counts, indicating unhandled 10-min timer interrupt.";
			sd.logStatus(msg, gps.dateTime());
		}
		portENTER_CRITICAL_ISR(&timerMux_base);
		_countInterrupts_60_min--;	// Interrupt handled.
		portEXIT_CRITICAL_ISR(&timerMux_base);
	}

	// ====================================================
	//  CHECK FOR NEW DAY
	if (day() > _oldDay || month() > _oldMonth || year() > _oldYear) {
		// NEW DAY. 
		// Save minima and maxima for previous day.
		processReadings_Day();
		_oldDay = day();
		_oldMonth = month();
		_oldYear = year();
		sd.logStatus("New day rollover.", gps.dateTime());
	}

	/// ==========  TEST FOR LOST WIFI CONNECTION  ========== //
	/*
	If WiFi is lost, we're screwed because the time
	to reconnect may throw of the sensor read timings.
	Just bite the bullet and take the time to reconnect,
	then recover.

	If WiFi was lost, the time to reconnect will cause
	the timer interrupt counts to increment beyond where
	they should have been handled.
	*/
	if (!_isDEBUG_BypassWifi) {
		if (WiFi.status() != WL_CONNECTED) {
			checkWifiConnection();
		}
	}

#if defined(VM_DEBUG)
	// Add delay for DEBUG.
	if (_isDEBUG_AddDelayInLoop) {
		vTaskDelay(_LOOP_DELAY_DEBUG_ms / portTICK_PERIOD_MS);
	}
#endif

	// Watch for excessive processing time in loop.
	if (millis() - _timeStart_Loop > LOOP_TIME_WARNING_THRESHOLD_MS) {
		String msg = "WARNING: Loop " + String(millis() - _timeStart_Loop) + "ms";
		sd.logStatus(msg, gps.dateTime());
	}
}
/******************************        END LOOP        **********************************/
/****************************************************************************************/
/****************************************************************************************/
