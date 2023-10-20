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
#include <AsyncTCP.h>

// Debugging flags
#include "DebugFlags.h"

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
#include "Utilities.h"
#include "GPSModule.h"
#include "SDCard.h"
#include "dataPoint.h"
#include "SensorData.h"
#include "WindSpeed2.h"
#include "WindDirection.h"

//#if defined(VM_DEBUG)
#include "Testing.h"			// DEBUG AND TESTING
#include "SensorSimulate.h"
//#endif

using namespace App_Settings;
using namespace Utilities;

/*
SensorData objects to average readings.
Wind speed handled by WindSpeed.
Wind direction handled by WindDirection.
*/

SensorData d_Temp_F;			// Temperature readings.
SensorData d_Pres_mb;			// Pressure readings.
SensorData d_Pres_seaLvl_mb;	// Pressure readings.
SensorData d_Temp_for_RH_C;		// Sensor temperature for pressure readings.
SensorData d_RH;				// Rel. humidity readings.
SensorData d_UVA;				// UVA readings.
SensorData d_UVB;				// UVB readings.
SensorData d_UVIndex;			// UV Index readings.
SensorData d_Insol;				// Insolaton readings.
SensorData d_IRSky_C;			// IR sky temperature readings.
SensorData d_fanRPM;			// Fan RPM readings.

WindSpeed windSpeed(
	DAVIS_SPEED_CAL_FACTOR,
	true,
	WIND_SPEED_NUMBER_IN_MOVING_AVG,
	WIND_SPEED_OUTLIER_DELTA);	// WindSpeed instance for wind.
SensorData windGust;
WindDirection windDir(VANE_OFFSET);	// WindDirection instance for wind.


//#if defined(VM_DEBUG)
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
//#endif

// Keep track of timer interrupts that trigger readings.
volatile int _countInterrupts_base = 0;		// Base timer interrupt count to trigger base sensor read.
volatile int _countInterrupts_10_min = 0;	// Base timer interrupt count to trigger 10-min averages.
volatile int _countInterrupts_60_min = 0;	// Base timer interrupt count to trigger 60-min averages.

// %%%%%%%%%%   STATUS FLAGS FOR DEVICES   %%%%%%%%%%%%%%%%
bool isGood_Temp = false;
bool isGood_PRH = false;
bool isGood_UV = false;
bool isGood_IR = false;
bool isGood_WindDir = false;
bool isGood_WindSpeed = false;
bool isGood_GPS = false;
bool isGood_PMS = false;
bool isGood_SDCard = false;
bool isGood_Solar = false;
bool isGood_LITTLEFS = false;
bool isGood_fan = false;
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// ==========   SD card module   ==================== //
const int SPI_CS_PIN = 5;	// CS pin for the SD card module
SDCard sd;		// SDCard instance.


// ==========   Async Web Server   ================== //
AsyncWebServer server(80);	// Async web server instance on port 80.
chartRequested _chart_request = CHART_NONE;	// Chart requested from server.


// ==========   u-blox NEO-6M GPS   ========================== //

// GPS module instance. 
GPSModule gps;

int _oldMonth = 0;		// Month of previous day.
int _oldDay = 0;		// Day of previous day.
int _oldYear = 0;		// Year of previous day.

const int RX2_PIN = 16;                 // UART2 U2_RXD
const int TX2_PIN = 17;                 // UART2 U2_RXD
#define SERIAL_CONFIGURATION SERIAL_8N1	// data, parity, and stop bits
const int GPS_BAUD_RATE = 9600;         // Beitian = 9600; Brian's = 38400; NEO-6M 9600


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
const int ONE_WIRE_PIN = 4;   // GPIO 4 pin for DS18B20
// Setup a oneWire instance to communicate with any OneWire devices.
OneWire oneWire(ONE_WIRE_PIN);
// Pass our oneWire reference to Dallas Temperature sensor.
DallasTemperature sensor_T(&oneWire);

/// <summary>
/// Returns temperature (F) from DS18B20 sensor.
/// </summary>
/// <returns></returns>
float reading_Temp_F_DS18B20() {
	sensor_T.requestTemperatures();	// Begin DS18B20 sensor read.
	float temp = sensor_T.getTempFByIndex(0);	// Retrieve reading
	return temp;
}

// ==========   Photovoltaic cell - solar radiation   ========================= //
const int SOLAR_PIN = 32;	// Photovoltaic cell uses GPIO 32 (Analog ADC1 CH4)

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

// ==========   PWM Fan for Radiation Shield  ======================== //
const int FAN_PWM_PIN = 27;				//  GPIO 27 - fan PWM control pin
const int FAN_SPEED_PIN = 35;			// Fan speed pin GPIO 35 (Digital Input with pullup).
const int FAN_PWM_FREQUENCY = 25000;	// 25kHz
const int FAN_PWM_CHANNEL = 0;
const int FAN_PWM_RESOLUTION = 8;
volatile unsigned long _fanHalfRots = 0;// count fan half-rotation (2 counts/cycle)

// HARDWARE INTERRUPT for fan tachometer switch that signals half-rotation
portMUX_TYPE hardwareMux_fan = portMUX_INITIALIZER_UNLOCKED;

// Function called by hardware interrupt when fan tachometer switch closes.
// It increments the half-rotation count, after accounting for switch debouncing.  XXX ???
void IRAM_ATTR ISR_onFanHalfRotation() {
	portENTER_CRITICAL_ISR(&hardwareMux_fan);
	_fanHalfRots++;
	portEXIT_CRITICAL_ISR(&hardwareMux_fan);
}

// ==========   Davis Anemometer 6410  ======================== //
/*
Davis RJ11 "Telephone" Plug: 6p4c
6 pins/4 conductors - but no connections to pins 1, 6
	1   na         not used            na
	2   Yellow     power in            VCC
	3   Green      wind vane pot.      GPIO 34 (analog in)
	4   Red        ground              GND
	5   Black      wind speed switch   GPIO 15 (digital in w/pullup)
	6   na         not used            na
*/

// ==========   Davis wind vane - wind direction   ================ //

const int WIND_VANE_PIN = 34;		// Wind vane is connected to GPIO 34 (Analog ADC1 CH6)

// ==========   Davis anemometer - wind speed  ==================== //



/*
 Wind speed is determined by counting anemometer
 rotations over a brief base measurement period.
 Each rotation triggers a hardware interrupt that
 increments _anem_Rotations by 1.
*/
const int WIND_SPEED_PIN = 15;					// GPIO 15 (Digital Input with pullup)
volatile unsigned int _anem_Rotations = 0;		// Count of anemometer rotations
unsigned long _lastDebounceTime = 0;			// Last millis when output pin was toggled
const unsigned int DEBOUNCE_TIMEOUT = 15;		// Debounce timeout (millisec)

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


// ==========   WIFI CONNECTIVITY   ================ //

WiFiMulti wifiMulti;	// WiFiMulti instance to connect to wifi.

/// <summary>
/// Connect to strongest WiFi access point. 
/// Will not return until connection succeeds 
/// or timeout is reached.
/// </summary>
/// <param name="timeout_sec">
/// Maximum number of seconds to try until failure.</param>
/// <returns>True if connection is successful.</returns>
bool wifiConnect(unsigned int timeout_sec) {
	unsigned long timeStart = millis();
	bool isConnected = false;
	// Try to connect for timeout_sec.
	while (
		wifiMulti.run(WIFI_CONNECT_TIMEOUT_SEC * 1000) != WL_CONNECTED
		&&
		millis() - timeStart < timeout_sec * 1000
		) {
		// Trying to connect ...
	}
	String msg = "WiFi connection duration ";
	msg += String((millis() - timeStart) / 1000.) + "s";
	sd.logStatus(msg);
	return  WiFi.isConnected();
}

/// <summary>
/// Check WiFi connection and reconnect if lost.
/// </summary>
/// <returns>True if WiFi is connected.</returns>
bool checkWifiConnection() {
	// XXX Note: Can also try WL_CONNECTION_LOST !!!
	// If WiFi is lost, reconnect.
	bool isConnected = true;
	if (WiFi.status() != WL_CONNECTED) {	// XXX THIS DID NOT WORK?!?!?!
		// Wifi not connected.
		unsigned long timeStart = millis();
		bool isResetTimerCounts = true;	// Flag to reset rotation count
		// Attempt to reconnect to wifi.
		isConnected = false;
		if (wifiConnect(WIFI_CONNECT_TIMEOUT_LOST_SEC)) {
			// Success.
			isConnected = true;
			// Log connection success.
			String msg = "Wifi re-connect successful after ";
			msg += String((millis() - timeStart) / 1000., 3) + "s";
			sd.logStatus(msg, gps.dateTime());
			sd.logStatus_indent(wifi_ConnectionInfo());
			// Print IP address to serial monitor.
			Serial.println("SERVER IP ADDRESS: " + WiFi.localIP().toString());
		}
		else {
			// Failure.
			isConnected = false;
			// Log connection failure.
			String msg = "Wifi connect FAILED after ";
			msg += String((millis() - timeStart) / 1000., 3) + "s";
			sd.logStatus(msg, gps.dateTime());
		}
		// Reset timer interrupt counts with delayed 
		// resets that occurred while reconnecting wifi.
		if (isResetTimerCounts) {
			resetTimerInterruptCounts();
			String msg = "Read cycle skipped after WiFi was lost.";
			sd.logStatus(msg, gps.dateTime());
			isResetTimerCounts = false;	// Reset flag.
		}
		return isConnected;
	}
}

/// <summary>
/// Returns string describing WiFi status.
/// </summary>
/// <returns>String describing WiFi status</returns>
String wifiConnect_Status() {
	/*
	Key to WiFi status enum:
		WiFi.status() values:
		WL_NO_SHIELD = 255,
		WL_IDLE_STATUS = 0,
		WL_NO_SSID_AVAIL = 1,
		WL_SCAN_COMPLETED = 2,
		WL_CONNECTED = 3,
		WL_CONNECT_FAILED = 4,
		WL_CONNECTION_LOST = 5,
		WL_DISCONNECTED = 6
	*/
	switch (WiFi.status())
	{
	case WL_NO_SSID_AVAIL:
		return "WiFi SSID not found";
	case WL_CONNECT_FAILED:
		return "WiFi connect failed";
	case WL_CONNECTION_LOST:
		return "WiFi Connection was lost";
	case WL_SCAN_COMPLETED:
		return "WiFi Scan is completed";
	case WL_DISCONNECTED:
		return "WiFi is disconnected";
	case WL_CONNECTED:
		return "WiFi is connected!";
	default:
		return "WiFi Status: " + WiFi.status();	// Integer value.
	}
}

/// <summary>
/// Adds credentials for WiFi access points.
/// </summary>
void wifiAddAccessPoints() {
	wifiMulti.addAP("RP-GL-24Ghz", "dew3pays");			// GLI.net
	wifiMulti.addAP("RPPhoto", "M##$e4you*");			// RP office
	wifiMulti.addAP("RP-router", "M##$e4you*");			// home
	wifiMulti.addAP("RP-HooToo", "dew3pays");			// HooToo portable
	wifiMulti.addAP("Vaughn Wireless", "redracoon1");	// Vaughn office
}

/// <summary>
/// Returns string listing connected WiFi SSID, RSSI, and IP adresss.
/// </summary>
/// <returns></returns>
String wifi_ConnectionInfo() {
	String s = "WiFi connection: SSID ";
	s += String(WiFi.SSID());
	s += ", RSSI " + String(WiFi.RSSI());
	s += ", IP address " + WiFi.localIP().toString();
	return s;
}

/// <summary>
/// Lists WiFi networks that are in range. 
/// </summary>
/// <returns>Number of networks found.</returns>
int wifiListNetworks() {
	int n = WiFi.scanNetworks();	// returns number of networks
	String msg = "Network scan complete.";
	sd.logStatus(msg, millis());
	if (n == 0) {
		sd.logStatus("No networks found.");
	}
	else {
		msg = "Found " + String(n) + " networks:";
		sd.logStatus(msg, millis());
		for (int i = 0; i < n; ++i) {
			// Print SSID and RSSI for each network found
			msg = String(WiFi.SSID(i));
			msg += "\t";
			msg += String(WiFi.RSSI(i));
			sd.logStatus_indent(msg);
		}
	}
	return n;
}

/// <summary>
/// Connects to the WiFi network.
/// </summary>
void wifiSetupAndConnect() {
	if (!_isDEBUG_BypassWifi) {
		// Specify WiFi credentials for router(s).
		wifiAddAccessPoints();
		// Connect to wifi.
		sd.logStatus("Connecting to Wifi.", gps.dateTime());
#if defined(VM_DEBUG)
		wifiListNetworks();
#endif
		if (wifiConnect(WIFI_CONNECT_TIMEOUT_LOST_SEC)) {
			sd.logStatus("Wifi connected.", gps.dateTime());
			sd.logStatus_indent(wifi_ConnectionInfo());
		}
		else {
			sd.logStatus("Wifi Connection FAILED.", gps.dateTime());
		}
	}
	else {
		// Bypassing wifi
		sd.logStatus("BYPASS WIFI", gps.dateTime());
	}
}

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

/// <summary>
/// Logs summary of devices status.
/// </summary>
void logDeviceStatus() {
	String msg = "DEVICE STATUS REPORT:";
	sd.logStatus(msg, gps.dateTime());
	msg = "WiFi connected - ";
	msg += bool_OK_Error(WiFi.isConnected());
	sd.logStatus_indent(msg);
	msg = "SD card - ";
	msg += bool_OK_Error(isGood_SDCard);
	sd.logStatus_indent(msg);
	msg = "LittleFS flash file system - ";
	msg += bool_OK_Error(isGood_LITTLEFS);
	sd.logStatus_indent(msg);
	msg = "GPS module - ???";
	sd.logStatus_indent(msg);
	msg = "Dallas temperature sensor - ";
	msg += bool_OK_Error(isGood_Temp);
	sd.logStatus_indent(msg);
	msg = "Radiation shield fan - ";
	msg += bool_OK_Error(isGood_fan);
	sd.logStatus_indent(msg);
	msg = "Pressure & RH sensor - ";
	msg += bool_OK_Error(isGood_PRH);
	sd.logStatus_indent(msg);
	msg = "Insolation sensor - ";
	msg += "???";
	sd.logStatus_indent(msg);
	msg = "UV sensor - ";
	msg += bool_OK_Error(isGood_UV);
	sd.logStatus_indent(msg);
	msg = "Wind direction sensor - ";
	msg += "???";
	sd.logStatus_indent(msg);
	msg = "Wind speed sensor - ";
	msg += "???";
	sd.logStatus_indent(msg);
	msg = "Time zone offset from GMT - ";
	msg += gps.timeZoneOffset();
	sd.logStatus_indent(msg);
	msg = "Is Daylight Time:  - ";
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
	if (_isDEBUG_addDummyDataList) {
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
/// Lists contents of directory.
/// </summary>
/// <param name="fs">File system.</param>
/// <param name="dirname">Path of the directory.</param>
/// <param name="levels">Number of levels to list.</param>
void listDirectory(fs::FS& fs, const char* dirname, uint8_t levels) {
	String msg = "Listing directory: " + String(dirname);
	sd.logStatus(msg);
	//Serial.printf("Listing directory: %s\r\n", dirname);
	File root = fs.open(dirname);
	if (!root) {
		sd.logStatus_indent("ERROR: Failed to open directory");
		return;
	}
	if (!root.isDirectory()) {
		sd.logStatus_indent("ERROR: Not a directory");
		return;
	}
	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			msg = "dir: " + String(file.name());
			sd.logStatus_indent(msg);
			//Serial.printf("\tdir: %s\n", file.name());
			if (levels) {
				listDirectory(fs, file.name(), levels - 1);	// Recursive.
			}
		}
		else {
			msg = "file: " + String(file.name());
			msg += " size: " + String(file.size() / 1000., 2) + " KB";
			sd.logStatus_indent(msg);
		}
		file = root.openNextFile();
	}
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
	if (isGood_UV) {
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
	if (isGood_UV) {
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
#if defined(VM_DEBUG)
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
#endif
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
	if (isGood_UV) {
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

/// <summary>
/// Initializes all sensors.
/// </summary>
void initializeSensors() {
	//  ---------------  BME280 P, RH sensor   ---------------
	// Initialize BME280 sensor.
	if (!sensor_PRH.begin(0x77)) {
		String msg = "WARNING: BME280 P/RH sensor not found.";
		sd.logStatus(msg, millis());
	}
	else {
		isGood_PRH = true;
		String msg = "BME280 P/RH sensor found.";
		sd.logStatus(msg, millis());
	}
	//  ---------------  DS18B20 T sensor   ---------------
	sensor_T.begin();
	// Find DS18B20 temperature.
	if (countOneWireDevices() < 1) {
		isGood_Temp = false;
		String msg = "WARNING: DS18B20 T sensor not found.";
		sd.logStatus(msg, millis());
	}
	else {
		isGood_Temp = true;
		String msg = "DS18B20 T sensor found.";
		sd.logStatus(msg, millis());
	}
	//  ---------------  VEML6075 UV sensor   ---------------
	// The VEML6075 begin returns true on success
	// or false on failure to communicate.
	if (sensor_UV.begin() == true) {
		isGood_UV = true;
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
		isGood_IR = false;
		String msg = "WARNING: MLX90614 IR sensor not found.";
		sd.logStatus(msg, millis());
	}
	else {
		isGood_IR = true;
		String msg = "MLX90614 IR sensor found.";
		sd.logStatus(msg, millis());
	}
	//  ---------------  Davis wind sensors   ---------------
	windDir.begin();	// Initialize WindDirection.
	String msg = "[ No connection test implemented for Davis anemometer. ]";
	sd.logStatus(msg, millis());
	isGood_WindDir = true;      // How can this be tested?? XXX
	isGood_WindSpeed = true;

	//  ---------------  Initialize LittleFS   ---------------
	if (!LittleFS.begin()) {
		String msg = "ERROR: LittleFS didn't mount.";
		sd.logStatus(msg, millis());
	}
	else {
		isGood_LITTLEFS = true;
		String msg = "LittleFS mounted.";
		sd.logStatus(msg, millis());
	}
	// Log used and available space.
	logLittleFsSpaceUsage();

	if (_isDEBUG_ListLittleFS) {
		listDirectory(LittleFS, "/", 5);
	}

	sd.logStatus("Device initialization complete.", millis());
	logDeviceStatus();
}

/// <summary>
/// Adds labels to SensorData objects.
/// </summary>
void sensorsAddLabels() {
	windSpeed.addLabels("Wind Speed", "Wind", "mph");
	windDir.addLabels("Wind direction", "Wind Dir", "", "&deg;");
	windGust.addLabels("Wind Gust", "Gust", "mph");
	d_Temp_F.addLabels("Temperature", "Temp", "F", "&deg;F");
	d_Pres_mb.addLabels("Pressure (abs)", "Pres (abs)", "mb");
	d_Pres_seaLvl_mb.addLabels("Pressure (SL)", "Pres (sl)", "mb");
	d_Temp_for_RH_C.addLabels("Temp for RH", "T for RH", "C", "&degC;");
	d_RH.addLabels("Rel. Humidity", "RH", "%", "&percnt;");
	d_IRSky_C.addLabels("Sky Temperature", "Sky Temp", "C", "&degC;");
	d_UVA.addLabels("UV A Radiation", "UV-A", "");
	d_UVB.addLabels("UV B Radiation", "UV-B", "");
	d_UVIndex.addLabels("UV Index", "UV Index", "");
	d_Insol.addLabels("Insolation", "Sun", "%", "&percnt;");
	d_fanRPM.addLabels("Aspirator Fan speedInstant", "Fan speedInstant", "rpm");
}


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

/// <summary>
/// Adds simulate wind sensor readings.
/// </summary>
void readWind_Simulate() {
	//#if defined(VM_DEBUG)
		//unsigned int rots = dummy_anemCount.linear(15, 0);				// simulate
	//unsigned int rots = dummy_anemCount.sawtooth(5, 0.1, 15);		// simulate
	//unsigned int rots = dummy_anemCount.sawtooth(rotsFromSpeed(10), rotsFromSpeed(0.2), rotsFromSpeed(15), rotsFromSpeed(12), 20, 1000);
	float target_speed = 15;
	float target_spikeSpeed = 15;
	float target_incSpeed = 0.1;
	unsigned int rots = dummy_anemCount.linear(rotsFromSpeed(target_speed), rotsFromSpeed(target_incSpeed), rotsFromSpeed(target_spikeSpeed), 50, 6);
	float speed = windSpeed.speedInstant(rots, BASE_PERIOD_SEC);	// Speed value
	dataPoint dpSpeed(now(), speed);
	windSpeed.addReading(dpSpeed);


	// Record any gusts. Use MOVING AVG of wind speed.
	//float avg = 99;
	float avg_moving = windSpeed.avgMoving();
	dataPoint dpGust = windSpeed.gust(dpSpeed, avg_moving);
	windGust.addReading(dpGust);


	// Read wind direction.
	float windAngle = dummy_windDir.sawtooth(90, 1, 360);
	windDir.addReading(now(), windAngle, dpSpeed.value);	// weighted by speed
	//#endif
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
	dataPoint dp;	// holds reading
	// Temperature.
	dp = dataPoint(now(), dummy_Temp_F.sawtooth(10, 0.02, 20));
	d_Temp_F.addReading(dp);
	// UV readings.
	dp = dataPoint(now(), dummy_UVA.linear(3, 0.1));
	d_UVA.addReading(dp);
	dp = dataPoint(now(), dummy_UVB.linear(3, 0.1));
	d_UVB.addReading(dp);
	dp = dataPoint(now(), dummy_UVIndex.sawtooth(0, 0.05, 10));
	d_UVIndex.addReading(dp);
	// P, RH
	dp = dataPoint(now(), dummy_RH.sawtooth(0, 0.05, 50));
	d_RH.addReading(dp);
	dp = dataPoint(now(), dummy_Pres_mb.linear(3, 0.1) / 100);
	d_Pres_mb.addReading(dp);			// Raw pressure in mb (hectopascals)
	dp = dataPoint(now(), dummy_RH.linear(10, 0.02));
	d_Temp_for_RH_C.addReading(dp);		// Temp (C) of P, RH sensor.
	// P adjusted to sea level.
	float psl = pressureAtSeaLevel(
		dummy_Pres_seaLvl_mb.linear(3, 0.1),
		gps.data.altitude(),
		d_Temp_for_RH_C.valueLastAdded());
	dp = dataPoint(now(), psl);
	d_Pres_seaLvl_mb.addReading(dp);
	// IR sky
	dp = dataPoint(now(), dummy_IRSky_C.sawtooth(0, 0.02, 10));
	d_IRSky_C.addReading(dp);
	// Insolation/
	float insol_norm = insol_norm_pct(dummy_Insol.linear(3, 0.1), INSOL_REFERENCE_MAX);
	dp = dataPoint(now(), insol_norm);
	d_Insol.addReading(dp);	// % Insolation
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
void processReadings_Day() {
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



/// <summary>
/// Test code to insert in setup for debugging.
/// </summary>
/// <param name="runTime_sec">Number of seconds to run.</param>
void testCodeForSetup(unsigned long runTime_sec) {
	//#if defined(VM_DEBUG)
	Serial.println(LINE_SEPARATOR);
	Serial.print("TEST in setup to run for "); Serial.print(runTime_sec); Serial.println(" sec/n");
	unsigned long timeStart = millis();
	/********************************/
	/* INSERT DEFINITIONS HERE.     */
#include "ListFunctions.h"

	/********************************/
	while (millis() < timeStart + runTime_sec * 1000)
	{
		/********************************/
		/* INSERT TEST CODE HERE.       */

		list<float> testList;
		float val = 10;
		for (size_t i = 0; i < 5; i++)
		{
			val += i;
			addToList(testList, val, 5);
		}
		float avg = listAverage(testList, 5);
		Serial.printf("list avg = %f", avg);


		/********************************/
	}
	Serial.println("TEST COMPLETE");
	Serial.println(LINE_SEPARATOR);
	while (true) {}	// infinite loop to halt
	//#endif
}

/// <summary>
/// Adds dummy data to SensorData instance lists.
/// </summary>
void addDummyData() {
	//#if defined(VM_DEBUG)
	Serial.printf("\naddDummyData now() = %li\n", now());
	d_Temp_F.addDummyData_10_min(65, -0.75, 12, 1765412100);
	d_Pres_mb.addDummyData_10_min(991, 1, 12, 1765412100);
	d_Pres_seaLvl_mb.addDummyData_10_min(991, 1, 12, 1765412100);
	// RE-WRITE THIS: d_RH.addDummyData_10_min(20, .5, 12, 1765412100);
	// RE-WRITE THIS: d_IRSky_C.addDummyData_10_min(-25, 0.5, 12, 1765412100);
	//windSpeed.addDummySpeedData_10_min(15, 0.5, 12, 1765412100);
//////////////	windSpeed.addDummyGustData_10_min(25, 2, 12, 1765412100);
	//	windDir.addDummyData_10_min(270, 5, 12, 1765412100);
	d_Insol.addDummyData_10_min(2700, 25, 12, 1765412100);
	d_UVIndex.addDummyData_10_min(0, 0.5, 12, 1765412100);

	d_Temp_F.addDummyData_60_min(65, 0.1, 12, 1765412100);
	d_Pres_mb.addDummyData_60_min(989, 1.5, 12, 1765412100);
	d_Pres_seaLvl_mb.addDummyData_60_min(991, 2, 12, 1765412100);
	d_RH.addDummyData_60_min(20, .5, 12, 1765412100);
	d_IRSky_C.addDummyData_60_min(-25, 0.5, 12, 1765412100);
	//windSpeed.addDummySpeedDataToAvg_60_min(15, 0.5, 12, 1765412100);
	//windSpeed.addDummyGustDataToAvg_60_min(25, 2, 12, 1765412100);
	//	windDir.addDummyData_60_min(270, 5, 12, 1765412100);
	d_Insol.addDummyData_60_min(2700, 25, 12, 1765412100);
	d_UVIndex.addDummyData_60_min(0, 0.5, 12, 1765412100);

	d_Temp_F.addDummyData_maxima(65, 0.1, 12, 1765412100);
	//#endif
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

	// Add labels to the SensorData objects.
	sensorsAddLabels();

	//  ==========  INITIALIZE SD CARD   ========== //
	// (Do this first - need SD card for logging.)
	sd.initialize(SPI_CS_PIN, _isDEBUG_BypassSDCard);
	// Begin status log entries to SD card.
	sd.logStatus();	// Empty line
	sd.logStatus(LINE_SEPARATOR_MAJOR);
	sd.logStatus("SETUP continues after SD card initialization.", gps.dateTime());
	sd.createFile(LOGFILE_PATH_DATA);
	sd.createFile(LOGFILE_PATH_STATUS);

	// Log the settings to the status file.
	logDebugStatus();
	logApp_Settings();

	//  ==========  INITIALIZE WIFI NETWORK   ========== //	
	wifiSetupAndConnect();


	//  ==========  INITIALIZE ASYNC WEB SERVER   ========== //	
	serverRouteHandler();	// Define routes for server requests.
	sd.logStatus("Async web server routes defined.", millis());
	server.begin();			// Start async web server.
	sd.logStatus("Async web server beginning.", millis());


	// ==========   INITIALIZE GPS AND SYNC TO GET TIME   ========== //
	// XXX  Need code to alter power of GPS!!!  XXX

	/* 	Format for setting s serial port:
		SerialObject.begin(baud-rate, protocol, RX pin, TX pin);
		(Wrong baud rate gives serial garbage.)
	*/
	// Connect to GPS
	gps.begin(GPS_BAUD_RATE, SERIAL_CONFIGURATION, RX2_PIN, TX2_PIN);
	sd.logStatus("Trying to connect to GPS.", millis());
	// Get time and location from GPS.
	// This code is BLOCKING until gps syncs.
	if (!gps.isSynced()) {
		gps.syncToGPS(sd, _isDEBUG_BypassGPS);
	}
	// Hold to determine when new day begins.
	_oldDay = day();
	_oldMonth = month();
	_oldYear = year();

#if defined(VM_DEBUG)
	////////  TESTING   ////////
	if (_isDEBUG_addDummyDataList) {
		addDummyData();
	}
	if (_isDEBUG_run_test_in_setup) {
		testCodeForSetup(200000);
	}
#endif

	// ==========  INITIALIZE SENSORS  ========== //
	initializeSensors();

	sd.logData(columnNames());	// Write column names to data log.
	sd.logStatus_indent("DATA COLUMNS:\t" + columnNames());	// Write column names to status log.

	/// ==========  CONFIGURE FAN PWM  ========== //
	ledcSetup(FAN_PWM_CHANNEL, FAN_PWM_FREQUENCY, FAN_PWM_RESOLUTION);
	// Attach the channel to the pin for PWM output.
	ledcAttachPin(FAN_PWM_PIN, FAN_PWM_CHANNEL);
	// Set fan speed using PWM.
	ledcWrite(FAN_PWM_CHANNEL, FAN_DUTY_PERCENT / 100. * 256);


	// Initialize interrupts after everything else has completed.

	// ==========  INITIALIZE ANEMOMETER HARDWARE INTERRUPT  ========== //
	// Anemometer hardware interrupt for detecting rotation.
	pinMode(WIND_SPEED_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(WIND_SPEED_PIN),
		ISR_onRotation_anem,
		FALLING);
	// Fan hardware interrupt to detect half-rotation.
	pinMode(FAN_SPEED_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(FAN_SPEED_PIN), ISR_onFanHalfRotation, FALLING);


	// ==========  INITIALIZE TIMER INTERRUPT  ========== //
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



unsigned int _timeEnd_Loop = 0;		//monitor loop timing
unsigned long _timeStart_Loop = 0;	//monitor loop timing

/****************************************************************************/
/***************************       LOOP      ********************************/
/****************************************************************************/
void loop() {
	_timeStart_Loop = millis();	// To monitor loop execution time.

	// Check for timer interrupts that were 
	// not handled during code delays.
	catchUnhandledBaseTimerInterrupts();

	/*
	 Read sensors or process data at intervals
	 determined from timer interrupt.
	*/

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
		// Check for unhandled.
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
	//    60-MIN INTERVAL.
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
		_countInterrupts_60_min = 0;	// Interrupt handled.
		portEXIT_CRITICAL_ISR(&timerMux_base);
	}

	// ====================================================
	// HAS DAY CHANGED?
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
	_timeEnd_Loop = millis() - _timeStart_Loop;
	if (_timeEnd_Loop > LOOP_TIME_WARNING_THRESHOLD_MS) {
		String msg = "WARNING: Loop " + String(_timeEnd_Loop) + "ms";
		sd.logStatus(msg, gps.dateTime());
	}
		}
/******************************        END LOOP        **********************************/
/****************************************************************************************/
/****************************************************************************************/
