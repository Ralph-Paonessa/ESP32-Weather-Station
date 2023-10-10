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
#include "WindSpeed.h"
#include "WindDirection.h"

//#if defined(VM_DEBUG)
#include "Testing.h"			// DEBUG AND TESTING
#include "SensorDummy.h"
//#endif

using namespace App_Settings;
using namespace Utilities;

/**************************************************************/
/*****************      DEBUGGING FLAGS      ******************/
/**************************************************************/

bool _isDEBUG_BypassGPS = true;			// Bypass gps syncing.
bool _isDEBUG_BypassWifi = false;		// Bypass WiFi connect.
bool _isDEBUG_BypassSDCard = true;		// Bypass SD card.
bool _isDEBUG_ListLittleFS = false;		// List contents of LittleFS.
bool _isDEBUG_BypassWebServer = false;	// Bypass Web Server.
bool _isDEBUG_Test_setup = false;		// Run only test code inserted in Setup.
bool _isDEBUG_Test_loop = false;		// Run test code inserted in Loop.
bool _isDEBUG_addDummyData = false;		// Add dummy data.
bool _isDEBUG_addDummyReadings = true;	// Add dummy sensor reading values.
bool _isDEBUG_AddDelayInLoop = false;	// Add delay in loop.
const int _LOOP_DELAY_DEBUG_ms = 100;	// Debug delay in loop, msec.
/************************************************************/

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


SensorDummy dummy_T;
SensorDummy dummy_IR;
SensorDummy dummy_Wind;


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
AsyncWebServer server(80);	// Async web server object on port 80.
chartRequested chart_request = CHART_NONE;	// Chart requested from server.


// ==========   u-blox NEO-6M GPS   ========================== //

// GPS module object. 
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
// Create BME280 object. I2C address 0x77.
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

WindDirection windDir(VANE_OFFSET);	// WindDirection object for wind.
const int WIND_VANE_PIN = 34;		// Wind vane is connected to GPIO 34 (Analog ADC1 CH6)

// ==========   Davis anemometer - wind speed  ==================== //

WindSpeed windSpeed(DAVIS_SPEED_CAL_FACTOR);	// WindSpeed object for wind.

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

WiFiMulti wifiMulti;	// WiFiMulti object to connect to wifi.

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
	String msg = "WiFi connected in ";
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
		Serial.println("\tDEBUG: BYPASS SD CARD");	// Can't log to SD!
	}
	if (_isDEBUG_BypassWebServer) {
		sd.logStatus_indent("BYPASS WEB SERVER");
	}
	if (_isDEBUG_Test_setup) {
		sd.logStatus_indent("RUN TEST CODE IN SETUP");
	}
	if (_isDEBUG_Test_loop) {
		sd.logStatus_indent("RUN TEST CODE IN LOOP");
	}
	if (_isDEBUG_addDummyData) {
		sd.logStatus_indent("ADD DUMMY DATA");
	}
	if (_isDEBUG_addDummyReadings) {
		sd.logStatus_indent("ADD DUMMY READINGS");
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
	msg = "GPS_DELAY_BETWEEN_CYCLES: " + String(GPS_DELAY_BETWEEN_CYCLES);
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
	s += "\t" + String(windSpeed.gust_10_min());
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
	s += "\t" + String(windSpeed.gust_10_min());
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

	Serial.print(windSpeed.gust_10_min(), 1);
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

// ==========   SERVER DYNAMIC HTML   ================ //

/// <summary>
/// Replaces %PLACEHOLDER% elements in 
/// files served from async web server.
/// </summary>
/// <param name="var">Placeholder identifier.</param>
/// <returns>String substituted for placeholder.</returns>
String processor(const String& var) {

	// Add CSS light style during daylight.
	if (var == "CSS_LIGHT_STYLE") {
		// Switch display theme when ambient light is,
		// detected by normalized insolation %.
		if (d_Insol.valueLastAdded() > 0.01) {
			return "<link href = ""style.light.min.css"" rel = ""stylesheet"" media = ""all"" type = ""text/css"" />";
		}
		else {
			return "";
		}
	}
	///  Weather data.  ///////////////////
	if (var == "LAST_READINGS_DATETIME") {
		return gps.dateTime();
	}
	if (var == "CURRENT_TIME") {
		return gps.time();
	}
	if (var == "WEEKDAY") {
		return gps.dayString();
	}
	if (var == "TEMPERATURE_F")
		return String(d_Temp_F.valueLastAdded(), 0);
	if (var == "WIND_SPEED") {
		return String(windSpeed.valueLastAdded(), 0);	// 10-min avg
	}
	if (var == "WIND_GUST") {
		return String(windSpeed.gust_10_min(), 0);
	}
	if (var == "WIND_DIRECTION") {
		return String(windDir.directionCardinal());		// avg since last cleared (<= 10 min)
	}
	if (var == "WIND_ANGLE") {
		return String(windDir.angleAvg_now(), 0);		// avg since last cleared (<= 10 min)
	}
	if (var == "GPS_ALTITUDE") {
		return String(gps.data.altitude(), 0);
	}
	if (var == "PRESSURE_MB_SL") {
		return String(d_Pres_seaLvl_mb.valueLastAdded(), 0);
	}
	if (var == "PRESSURE_MB_ABS") {
		return String(d_Pres_mb.valueLastAdded(), 0);
	}
	if (var == "WATER_BOILING_POINT") {
		return String(waterBoilingPoint_F(d_Pres_mb.valueLastAdded()), 0);
	}
	if (var == "INSOLATION_PERCENT") {
		return String(d_Insol.valueLastAdded(), 0);
	}
	if (var == "REL_HUMIDITY") {
		return String(d_RH.valueLastAdded(), 0);
	}
	if (var == "UV_A") {
		if (isGood_UV) {
			return String(d_UVA.valueLastAdded(), 0);
		}
		else {
			return String("na");
		}
	}
	if (var == "UV_B") {
		if (isGood_UV) {
			return String(d_UVB.valueLastAdded(), 0);
		}
		else {
			return String("na");
		}
	}
	if (var == "UV_INDEX") {
		if (isGood_UV) {
			return String(d_UVIndex.valueLastAdded(), 1);
		}
		else {
			return String("na");
		}
	}
	if (var == "IR_T_SKY") {
		return String(d_IRSky_C.valueLastAdded(), 0);
	}


	///  Weather data daily maxima.  ///////////////////

	if (var == "TEMPERATURE_F_HI") {
		return String(d_Temp_F.max().value, 0);
	}
	if (var == "WIND_SPEED_HI") {
		return String(windSpeed.max().value, 0);
	}
	if (var == "WIND_GUST_HI") {
		return String(windSpeed.max().value, 0);
	}
	if (var == "WIND_ANGLE_HI") {
		return "??";		// avg since last cleared (<= 10 min)
	}
	if (var == "PRESSURE_MB_SL_HI") {
		return String(d_Pres_seaLvl_mb.max().value, 0);
	}
	if (var == "INSOLATION_PERCENT_HI") {
		return String(d_Insol.max().value, 0);
	}
	if (var == "REL_HUMIDITY_HI") {
		return String(d_RH.max().value, 0);
	}
	if (var == "UV_A_HI") {
		if (isGood_UV) {
			return String(d_UVA.max().value, 0);
		}
		else {
			return String("na");
		}
	}
	if (var == "UV_B_HI") {
		if (isGood_UV) {
			return String(d_UVB.max().value, 0);
		}
		else {
			return String("na");
		}
	}
	if (var == "UV_INDEX_HI") {
		if (isGood_UV) {
			return String(d_UVIndex.max().value, 1);
		}
		else {
			return String("na");
		}
	}
	if (var == "IR_T_SKY_HI") {
		return String(d_IRSky_C.max().value, 0);
	}


	///  Weather data daily minima.  ///////////////////

	if (var == "TEMPERATURE_F_LO") {
		return String(d_Temp_F.max().value, 0);
	}
	if (var == "WIND_SPEED_LO") {
		return String(windSpeed.max().value, 0);	// 10-min avg
	}
	if (var == "WIND_GUST_LO") {
		return String(windSpeed.max().value, 0);
	}
	if (var == "WIND_ANGLE_LO") {
		return "??";		// avg since last cleared (<= 10 min)
	}
	if (var == "PRESSURE_MB_SL_LO") {
		return String(d_Pres_seaLvl_mb.max().value, 0);
	}
	if (var == "REL_HUMIDITY_LO") {
		return String(d_RH.max().value, 0);
	}
	if (var == "IR_T_SKY_LO") {
		return String(d_IRSky_C.max().value, 0);
	}



	///  GPS info.   ////////////////////////

	if (var == "GPS_IS_SYNCED") {
		if (gps.isSynced())
			return String("Synced");
		else
			return String("Not Synced");
	}
	if (var == "GPS_LOCATIONS_UPDATE_COUNTER") {
		return String(gps.cyclesCount());
	}
	if (var == "GPS_LATITUDE") {
		return String(gps.data.latitude(), 6);
	}
	if (var == "GPS_LONGITUDE") {
		return String(gps.data.longitude(), 6);
	}
	if (var == "GPS_ALTITUDE") {
		return String(gps.data.altitude());
	}
	if (var == "GPS_DATE") {
		return String(gps.date_UTC_GPS());
	}
	if (var == "GPS_TIME") {
		return String(gps.time_UTC_GPS());
	}
	if (var == "GPS_TIME_ZONE") {
		return String(UTC_OFFSET_HOURS);
	}
	if (var == "GPS_DAYLIGHT_TIME_USED") {
		return bool_Yes_No(IS_DAYLIGHT_TIME);
	}
	if (var == "GPS_HDOP") {
		return String(gps.data.HDOP() / 100.);
	}
	if (var == "GPS_SATELLITES") {
		return String(gps.data.satellites());
	}
	if (var == "ELAPSED_TIME_STRING") {
		return   String(gps.data.timeToSync_sec(), 2);
	}
	if (var == "FAN_RPM") {
		return String(d_fanRPM.valueLastAdded());
	}

	/// CHART FIELDS  //////////////

	if (var == "TIME_OFFSET_HOURS") {
		if (IS_DAYLIGHT_TIME) {
			return String(UTC_OFFSET_HOURS + 1);
		}
		else {
			return  String(UTC_OFFSET_HOURS);
		}
	}
	if (var == "CHART_Y_AXIS_LABEL") {
		// Based on chart requested.
		switch (chart_request)
		{
		case CHART_NONE:
			return "Chart not specified!";
		case CHART_INSOLATION:
			return String(d_Insol.label() + ", " + d_Insol.units_html());
		case CHART_IR_SKY:
			return String(d_IRSky_C.label() + ", " + d_IRSky_C.units_html());
		case CHART_TEMPERATURE_F:
			return String(d_Temp_F.label() + ", " + d_Temp_F.units_html());
		case CHART_PRESSURE_SEA_LEVEL:
			return String(d_Pres_seaLvl_mb.label() + ", " + d_Pres_seaLvl_mb.units());
		case CHART_RELATIVE_HUMIDITY:
			return String(d_RH.label() + ", " + d_RH.units_html());
		case CHART_UV_INDEX:
			return String(d_UVIndex.label() + ", " + d_UVIndex.units());
		case CHART_WIND_DIRECTION:
			return String(windDir.label() + ", " + windDir.units_html());
		case CHART_WIND_SPEED:
			return String(windSpeed.label() + ", " + windSpeed.units());
		case CHART_WIND_GUST:
			return String("Wind Gusts, " + windSpeed.units());
		default:
			return "Chart not found";
		}
	}
	if (var == "CHART_TITLE") {
		// Based on chart requested.
		switch (chart_request)
		{
		case CHART_NONE:
			return "Chart not specified!";
		case CHART_INSOLATION:
			return String(d_Insol.label());
		case CHART_IR_SKY:
			return String(d_IRSky_C.label());
		case CHART_TEMPERATURE_F:
			return String(d_Temp_F.label());
		case CHART_PRESSURE_SEA_LEVEL:
			return String(d_Pres_seaLvl_mb.label());
		case CHART_RELATIVE_HUMIDITY:
			return String(d_RH.label());
		case CHART_UV_INDEX:
			return String(d_UVIndex.label());
		case CHART_WIND_DIRECTION:
			return String(windDir.label());
		case CHART_WIND_SPEED:
			return String(windSpeed.label());
		case CHART_WIND_GUST:
			return "Wind Gusts";
		}
	}
	// Y axis min.
	if (var == "Y_MIN") {
		// Based on chart requested.
		switch (chart_request)
		{
		case CHART_NONE:
			return "min: -500";
		case CHART_INSOLATION:
			return "min: 0";
		case CHART_IR_SKY:
			return "min: -50";
		case CHART_TEMPERATURE_F:
			return "min: 0";
		case CHART_PRESSURE_SEA_LEVEL:
			return "min: 950";
		case CHART_RELATIVE_HUMIDITY:
			return "min: 0";
		case CHART_UV_INDEX:
			return "min: 0";
		case CHART_WIND_DIRECTION:
			return "min: 0";
		case CHART_WIND_SPEED:
			return "min: 0";
		case CHART_WIND_GUST:
			return "min: 0";
		default:
			return "min: -2000";
		}
	}
	// Y axis max.
	if (var == "Y_MAX") {
		// Based on chart requested.
		switch (chart_request)
		{
		case CHART_NONE:
			return ", max: 500";
		case CHART_INSOLATION:
			return ", max: 100";
		case CHART_IR_SKY:
			return ", max: 50";
		case CHART_TEMPERATURE_F:
			return ", max: 100";
		case CHART_PRESSURE_SEA_LEVEL:
			return ", max: 1050";
		case CHART_RELATIVE_HUMIDITY:
			return ", max: 100";
		case CHART_UV_INDEX:
			return ", max: 20";
		case CHART_WIND_DIRECTION:
			return ", max: 360";
		case CHART_WIND_SPEED:
			return ", max: 50";
		case CHART_WIND_GUST:
			return ", max: 50";
		default:
			return ", max: 2000";
		}
	}
	// Y axis tick amount.
	if (var == "Y_TICK_AMOUNT") {
		// Based on chart requested.
		switch (chart_request)
		{
		case CHART_IR_SKY:
			return ", tickAmount: 5";
		case CHART_TEMPERATURE_F:
			return ", tickAmount: 5";
		case CHART_PRESSURE_SEA_LEVEL:
			return ", tickAmount: 3";
		case CHART_WIND_DIRECTION:
			return ", tickAmount: 2";
		case CHART_WIND_SPEED:
			return ", tickAmount: 6";
		case CHART_WIND_GUST:
			return ", tickAmount: 6";
		default:
			return "";
		}
	}

	///   Fan   //////////////
	if (var == "FAN_RPM") {
		return String(d_fanRPM.valueLastAdded());
	}
	return var + String(" not found");
}

/// <summary>
/// Defines uri routes for async web server.
/// </summary>
void serverRouteHandler() {
	/*
	Configure url routes where server will be listening. Route in url
	is: "http://[ IP Address ][Route]",	such as [Route]= "/data".

	Serving files from both LittleFS (flash memory) and SD (SD card).

	STATIC FILES:

		"Static" files are generally files that are "not changed
		by the server" such as css, js, and non-dynamic html pages.

		Usage:
			Format is server.serveStatic([ Route ], [ File system ], [ File path ]);

		Example:
		To serve the file "/dir/page.htm" when request url is "/page.htm":

			server.serveStatic("/page.htm", LittleFS, "/dir/page.htm");

	DYNAMIC FILES:

		Files can contain templates of the form	%REPLACE_ME% that are
		replaced by a processor function attached to the request.

		server.on adds a new instance of AsyncStaticWebHandler
		to the server to handle the specified file.

		(Note: ESPAsyncWebServer ReadMe says you can use template processor
		with "static" files. This seems contradictory; here, I refer to
		them as dynamic files.)

		Usage:
			server.on([ Route ], HTTP_GET, [](AsyncWebServerRequest* request)
				{
				request->send([ File system ], [ File path ] , [ Mime type ], [ isDownload ], [ template processor ]);
				});

		Example:
			server.on("/Admin", HTTP_GET, [](AsyncWebServerRequest* request)
				{
				request->send(LittleFS, "/html/Admin.html", "text/html", false, processor);
				});

		IMPORTANT: false is for bool download; otherwise browser will
		download file instead of display it!


	SETTING CACHE FOR STATIC FILES:

		Usage:
			server.serveStatic([Route], [File system], [File path]).setCacheControl("max-age=[seconds]");

		Example:
			server.serveStatic("/highcharts.css", LittleFS, "/css/highcharts.css").setCacheControl("max-age=864000");

			10 days = 864,000 seconds
			 2 days = 172,800 seconds
			 1 day  =  86,400 seconds
	*/

#if defined(VM_DEBUG)
	if (!_isDEBUG_BypassWebServer) {
#endif
		// Set cache for static files.
		// css
		server.serveStatic("/highcharts.css", LittleFS, "/css/highcharts.css").setCacheControl("max-age=864000");
		server.serveStatic("/highcharts-custom.css", LittleFS, "/css/highcharts-custom.css").setCacheControl("max-age=864000");
		server.serveStatic("/style.light.min.css", LittleFS, "/css/style.light.min.css").setCacheControl("max-age=864000");
		server.serveStatic("/style.min.css", LittleFS, "/css/style.min.css").setCacheControl("max-age=864000");
		// js
		server.serveStatic("/highcharts.js", LittleFS, "/js/highcharts.js").setCacheControl("max-age=864000");
		server.serveStatic("/chart.js", LittleFS, "/js/chart.js").setCacheControl("max-age=864000");
		// img
		server.serveStatic("/chart-icon.png", LittleFS, "/img/chart-icon-red-150px.png").setCacheControl("max-age=864000");
		server.serveStatic("/home-icon.png", LittleFS, "/img/home-icon-red-150px.png").setCacheControl("max-age=864000");
		server.serveStatic("/img/loading.gif", LittleFS, "/img/loading.gif").setCacheControl("max-age=864000");
		server.serveStatic("/favicon-32.png", LittleFS, "/img/favicon-32.png").setCacheControl("max-age=864000");
		server.serveStatic("/favicon-32.180", LittleFS, "/img/favicon-32.180").setCacheControl("max-age=864000");
		// html 
		// html pages are dynamic and can't be cached.

		/*****  WEB PAGES.  *****/

		// Default.
		server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/html/sensors.html", "text/html", false, processor);
			});

		// Summary data.
		server.on("/summary", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/html/summary.html", "text/html", false, processor);
			});

		// GPS info.
		server.on("/gps", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/html/gps.html", "text/html", false, processor);
			});

		// Admin page.
		server.on("/Admin", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/html/Admin.html", "text/html", false, processor);
			});

		// Log file from SD card.
		server.on("/log", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(SD, "/log.txt", "text/plain");
			});

		// Data file from SD card.
		server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(SD, "/data.txt", "text/plain");
			});

		/*****  GRAPH PAGES.  *****/

		// Pages use javascript to get data asynchronously.

		// Temperature graph page.
		server.on("/chart_T", HTTP_GET, [](AsyncWebServerRequest* request) {
			chart_request = CHART_TEMPERATURE_F;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Wind speed graph page.
		server.on("/chart_W", HTTP_GET, [](AsyncWebServerRequest* request) {
			chart_request = CHART_WIND_SPEED;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Wind gust graph page.
		server.on("/chart_Wgst", HTTP_GET, [](AsyncWebServerRequest* request) {
			chart_request = CHART_WIND_GUST;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Wind direction graph page.
		server.on("/chart_Wdir", HTTP_GET, [](AsyncWebServerRequest* request) {
			chart_request = CHART_WIND_DIRECTION;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Pressure (sea level) graph page.
		server.on("/chart_P", HTTP_GET, [](AsyncWebServerRequest* request) {
			chart_request = CHART_PRESSURE_SEA_LEVEL;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Relative Humidity graph page.
		server.on("/chart_RH", HTTP_GET, [](AsyncWebServerRequest* request) {
			chart_request = CHART_RELATIVE_HUMIDITY;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Sky Infrared graph page.
		server.on("/chart_IR", HTTP_GET, [](AsyncWebServerRequest* request) {
			chart_request = CHART_IR_SKY;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// UV Index graph page.
		server.on("/chart_UVIndex", HTTP_GET, [](AsyncWebServerRequest* request) {
			chart_request = CHART_UV_INDEX;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		// Insolation graph page.
		server.on("/chart_Insol", HTTP_GET, [](AsyncWebServerRequest* request) {
			chart_request = CHART_INSOLATION;
			request->send(LittleFS, "/html/chart.html", "text/html", false, processor);
			});

		/*****  Images.  *****/

		server.on("/favicon-32.png", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/img/favicon-32.png", "image/png");
			});
		server.on("/favicon-180.png", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/img/favicon-180.png", "image/png");
			});
		server.on("/chart-icon.png", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/img/chart-icon-red-150px.png", "image/png");
			});
		server.on("/home-icon.png", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/img/home-icon-red-150px.png", "image/png");
			});
		server.on("/img/loading.gif", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(LittleFS, "/img/loading.gif", "image/gif");
			});

		/*****  CSS and Javascript.  *****/

		// CSS style sheet.
		server.on("/style.min.css",
			HTTP_GET,
			[](AsyncWebServerRequest* request) {
				request->send(LittleFS, "/css/style.min.css", "text/css");
			});
		// CSS light-theme style sheet.
		server.on("/style.light.min.css",
			HTTP_GET,
			[](AsyncWebServerRequest* request) {
				request->send(LittleFS, "/css/style.light.min.css", "text/css");
			});
		// Highcharts css style sheet.
		server.on("/highcharts.css",
			HTTP_GET,
			[](AsyncWebServerRequest* request) {
				request->send(LittleFS, "/css/highcharts.css", "text/css");
			});
		// Highcharts customized css style sheet.
		server.on("/highcharts-custom.css",
			HTTP_GET,
			[](AsyncWebServerRequest* request) {
				request->send(LittleFS, "/css/highcharts-custom.css", "text/css");
			});
		// highcharts javascript file.
		server.on("/highcharts.js",
			HTTP_GET,
			[](AsyncWebServerRequest* request) {
				request->send(LittleFS, "/js/highcharts.js", "text/javascript");
			});
		// highcharts custom javascript file.
		server.on("/chart.js",
			HTTP_GET,
			[](AsyncWebServerRequest* request) {
				request->send(LittleFS, "/js/chart.js", "text/javascript");
			});

		/*****  DATA SOURCES FOR GRAPHS  *****/
		/*
		 Send string with data asynchronously to html
		 page where Javascript parses and plots the data.
		*/

		// 10-min charts
		server.on("/data_10", HTTP_GET,
			[](AsyncWebServerRequest* request) {
				// Which chart?
				switch (chart_request)
				{
				case CHART_NONE:
					request->send_P(200, "text/plain", "");
					break;
				case CHART_INSOLATION:
					request->send_P(200, "text/plain", d_Insol.data_10_min_string_delim(false, 0).c_str());
					break;
				case CHART_IR_SKY:
					request->send_P(200, "text/plain", d_IRSky_C.data_10_min_string_delim(false, 0).c_str());
					break;
				case CHART_TEMPERATURE_F:
					request->send_P(200, "text/plain", d_Temp_F.data_10_min_string_delim(false, 0).c_str());
					break;
				case CHART_PRESSURE_SEA_LEVEL:
					request->send_P(200, "text/plain", d_Pres_seaLvl_mb.data_10_min_string_delim(false, 0).c_str());
					break;
				case CHART_RELATIVE_HUMIDITY:
					request->send_P(200, "text/plain", d_RH.data_10_min_string_delim(false, 0).c_str());
					break;
				case CHART_UV_INDEX:
					request->send_P(200, "text/plain", d_UVIndex.data_10_min_string_delim(false, 1).c_str());
					break;
				case CHART_WIND_DIRECTION:
					request->send_P(200, "text/plain", windDir.data_10_min_string_delim(true, 0).c_str());
					break;
				case CHART_WIND_SPEED:
					request->send_P(200, "text/plain", windSpeed.data_10_min_string_delim(false, 0).c_str());
					break;
				case CHART_WIND_GUST:
					request->send_P(200, "text/plain", windSpeed.gusts_10_min_string_delim(true, 0).c_str());
					break;
				default:
					request->send_P(200, "text/plain", "");
					break;
				}
			});

		// 60-min charts
		server.on("/data_60", HTTP_GET,
			[](AsyncWebServerRequest* request) {
				// Which chart?
				switch (chart_request)
				{
				case CHART_NONE:
					request->send_P(200, "text/plain", "");
					break;
				case CHART_INSOLATION:
					request->send_P(200, "text/plain", d_Insol.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_IR_SKY:
					request->send_P(200, "text/plain", d_IRSky_C.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_TEMPERATURE_F:
					request->send_P(200, "text/plain", d_Temp_F.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_PRESSURE_SEA_LEVEL:
					request->send_P(200, "text/plain", d_Pres_seaLvl_mb.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_RELATIVE_HUMIDITY:
					request->send_P(200, "text/plain", d_RH.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_UV_INDEX:
					request->send_P(200, "text/plain", d_UVIndex.data_60_min_string_delim(false, 1).c_str());
					break;
				case CHART_WIND_DIRECTION:
					request->send_P(200, "text/plain", windDir.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_WIND_SPEED:
					request->send_P(200, "text/plain", windSpeed.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_WIND_GUST:
					request->send_P(200, "text/plain", windSpeed.gusts_60_min_string_delim(true, 0).c_str());
					break;
				default:
					request->send_P(200, "text/plain", "");
					break;
				}
			});


		// 60-min charts
		server.on("/data_60", HTTP_GET,
			[](AsyncWebServerRequest* request) {
				// Which chart?
				switch (chart_request)
				{
				case CHART_NONE:
					request->send_P(200, "text/plain", "");
					break;
				case CHART_INSOLATION:
					request->send_P(200, "text/plain", d_Insol.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_IR_SKY:
					request->send_P(200, "text/plain", d_IRSky_C.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_TEMPERATURE_F:
					request->send_P(200, "text/plain", d_Temp_F.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_PRESSURE_SEA_LEVEL:
					request->send_P(200, "text/plain", d_Pres_seaLvl_mb.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_RELATIVE_HUMIDITY:
					request->send_P(200, "text/plain", d_RH.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_UV_INDEX:
					request->send_P(200, "text/plain", d_UVIndex.data_60_min_string_delim(false, 1).c_str());
					break;
				case CHART_WIND_DIRECTION:
					request->send_P(200, "text/plain", windDir.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_WIND_SPEED:
					request->send_P(200, "text/plain", windSpeed.data_60_min_string_delim(false, 0).c_str());
					break;
				case CHART_WIND_GUST:
					request->send_P(200, "text/plain", windSpeed.gusts_60_min_string_delim(true, 0).c_str());
					break;
				default:
					request->send_P(200, "text/plain", "");
					break;
				}
			});





		// Daily maxima charts.
		server.on("/data_max", HTTP_GET,
			[](AsyncWebServerRequest* request) {
				// Which chart?
				switch (chart_request)
				{
				case CHART_NONE:
					request->send_P(200, "text/plain", "");
					break;
				case CHART_INSOLATION:
					request->send_P(200, "text/plain", d_Insol.maxima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_IR_SKY:
					request->send_P(200, "text/plain", d_IRSky_C.maxima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_TEMPERATURE_F:
					//request->send_P(200, "text/plain", d_Temp_F.maxima_daily_string_delim(false, 0).c_str());
					request->send_P(200, "text/plain", d_Temp_F.maxima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_PRESSURE_SEA_LEVEL:
					request->send_P(200, "text/plain", d_Pres_seaLvl_mb.maxima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_RELATIVE_HUMIDITY:
					request->send_P(200, "text/plain", d_RH.maxima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_UV_INDEX:
					request->send_P(200, "text/plain", d_UVIndex.maxima_daily_string_delim(false, 1).c_str());
					break;
				case CHART_WIND_DIRECTION:
					request->send_P(200, "text/plain", windDir.maxima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_WIND_SPEED:
					request->send_P(200, "text/plain", windSpeed.maxima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_WIND_GUST:
					request->send_P(200, "text/plain", windSpeed.maxima_daily_string_delim(true, 0).c_str());
					break;
				default:
					request->send_P(200, "text/plain", "");
					break;
				}
			});


		// Daily minima charts.
		server.on("/data_min", HTTP_GET,
			[](AsyncWebServerRequest* request) {
				// Which chart?
				switch (chart_request)
				{
				case CHART_NONE:
					request->send_P(200, "text/plain", "");
					break;
				case CHART_INSOLATION:
					request->send_P(200, "text/plain", d_Insol.minima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_IR_SKY:
					request->send_P(200, "text/plain", d_IRSky_C.minima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_TEMPERATURE_F:
					//request->send_P(200, "text/plain", d_Temp_F.minima_daily_string_delim(false, 0).c_str());
					request->send_P(200, "text/plain", d_Temp_F.minima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_PRESSURE_SEA_LEVEL:
					request->send_P(200, "text/plain", d_Pres_seaLvl_mb.minima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_RELATIVE_HUMIDITY:
					request->send_P(200, "text/plain", d_RH.minima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_UV_INDEX:
					request->send_P(200, "text/plain", d_UVIndex.minima_daily_string_delim(false, 1).c_str());
					break;
				case CHART_WIND_DIRECTION:
					request->send_P(200, "text/plain", windDir.minima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_WIND_SPEED:
					request->send_P(200, "text/plain", windSpeed.minima_daily_string_delim(false, 0).c_str());
					break;
				case CHART_WIND_GUST:
					request->send_P(200, "text/plain", windSpeed.minima_daily_string_delim(true, 0).c_str());
					break;
				default:
					request->send_P(200, "text/plain", "");
					break;
				}
			});





#if defined(VM_DEBUG)
	}
	else {
		Serial.println("BYPASSING WEB SERVER INITIALIZATION");
	}
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

/*
***   HOW SENSORS ARE READ AND LISTS ARE COMPILED.   ***

Frequent readings for most sensors are passed directly to
SensorData objects where they are averaged and assembled
into lists of data vs time for 10-min, 60-min, and 12-hr
periods.

Wind readings require special handling because of the need
to detect sudden variations and extremes (gusts) over a brief
period (ideally ca. 2.5 - 5 seconds).

Wind speed is handled by Anemometer::WindSpeed.

WindDirection is first handled by Anemometer::WindDirection, which
averages the wind direction vectors. The average direction
over 10 minutes is then passed to a SensorData object at
10-minute intervals to be held in lists of data vs time.

XXX
NOTE: THIS METHOD AVERAGES THE WIND DIRECTION FOR 10
MINUTES AND SAVES ONLY THAT 10-MIN AVERAGE. IS THAT TOO
SHORT A PERIOD, SMEARING THE DIRECTION? ALSO, SHOULD
DIRECTION BE WEIGHTED BY SPEED?? E.G., SHOULD A 20 MPH N
WIND FOR 5 MINUTES COUNT AS MUCH AS A SUBSEQUENT 1 MPH E
WIND FOR 5 MINUTES??
XXX
*/

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
/// Adds the specified value to a dummy sensor reading.
/// </summary>
/// <param name="val">Value to add.</param>
void readSensors_dummy(float val) {
	// Temperature.
	d_Temp_F.addReading(now(), val);	// temperature
}

/// <summary>
/// Reads and saves wind speed, gusts, and direction.
/// </summary>
void readWind() {
	// Read wind speed.
	/*
		XXX NOTE: Can _anem_Rotations still increase after
		timer interrupt signals BASE_PERIOD_SEC, while other
		processing delays the time until

			windSpeed.addReading(now(), _anem_Rotations)

		Should _anem_Rotations be held in another variable
		immediately when BASE_PERIOD_SEC is reached???
	*/
	int rots = _anem_Rotations;

	windSpeed.addReading(now(), _anem_Rotations);
	float speed = windSpeed.speedInstant(_anem_Rotations, BASE_PERIOD_SEC);

	// Read wind direction (average over 0-360 deg).
	/*
	Wind direction should be weighted by speed!
	If the direction is
		90 at 20 mph for 5 min
		180 at 2 mph for 5 min
	the direction shouldn't be a straight avg = 135.
	It should be weighted more heavily towards 90, where
	most of the wind flow was!
	ALSO, direction stuck at X when wind stops blowing
	should not contribute to direction -- or direction
	should be nothing!
		if (speed < threshold), direction = nothing
	*/

	float windAngle = windAngleReading();
	windDir.addReading(now(), windAngle, speed);

	// Reset hardware interrupt count.
	portENTER_CRITICAL_ISR(&hardwareMux_anem);
	_anem_Rotations = 0;	// Reset anemometer count.
	portEXIT_CRITICAL_ISR(&hardwareMux_anem);
}

/// <summary>
/// Reads and saves fan speed.
/// </summary>
void readFan() {
	// Get fan speed.
	d_fanRPM.addReading(now(), fanRPM(_fanHalfRots, BASE_PERIOD_SEC));
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
	// Temperature.
	d_Temp_F.addReading(now(), reading_Temp_F_DS18B20());	// temperature
	// UV readings.
	d_UVA.addReading(now(), sensor_UV.uva());
	d_UVB.addReading(now(), sensor_UV.uvb());
	d_UVIndex.addReading(now(), sensor_UV.index());
	// P, RH
	d_RH.addReading(now(), sensor_PRH.readHumidity());				// RH.
	d_Pres_mb.addReading(now(), sensor_PRH.readPressure() / 100);	// Raw pressure in mb (hectopascals)
	d_Temp_for_RH_C.addReading(now(), sensor_PRH.readTemperature());// Temp (C) of P, RH sensor.
	float psl = pressureAtSeaLevel(
		d_Pres_mb.valueLastAdded(),
		gps.data.altitude(),
		d_Temp_for_RH_C.valueLastAdded());
	d_Pres_seaLvl_mb.addReading(now(), psl);
	// IR sky
	d_IRSky_C.addReading(now(), sensor_IR.readObjectTempC());
	// Insolation/
	float insol_norm = insol_norm_pct(readInsol_mV(), INSOL_REFERENCE_MAX);
	d_Insol.addReading(now(), insol_norm);							// % Insolation
	unsigned int timeEnd = millis() - timeStart;
}

/// <summary>
/// Reads and saves data from sensors.
/// </summary>
void readSensors(bool isAddDummyReadings) {

	if (!isAddDummyReadings)
	{
		readSensors();
	}
	else {
		float dumVal = 0;
		unsigned int readsIn_10_min = 600 / BASE_PERIOD_SEC;
		// In 10-min, want to rise by 30. Values read every 4 sec.
		float increment = 30. / readsIn_10_min;
		//dumVal = dummy_T.risingVal(5, 90. / seconds);
		d_Temp_F.addReading(now(), dummy_T.risingVal(3, 0.1));

		d_IRSky_C.addReading(now(), dummy_IR.risingVal(-25, 0.005));
	}
}

/// <summary>
/// Saves 10-min averages of all sensor data 
/// to lists.
/// </summary>
void processReadings_10_min() {
	windSpeed.process_data_10_min();
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
/// Test code that can be inserted for debugging.
/// </summary>
void addTestCodeHere() {
#if defined(VM_DEBUG)
	/////////  TESTING   /////////////
	Serial.println(LINE_SEPARATOR);
	Serial.println("TEST in setup start\n");
	/*******************************

		INSERT TEST CODE HERE.

	********************************/
	Serial.println("TEST COMPLETE");
	Serial.println(LINE_SEPARATOR);
	while (true) {}	// infinite loop to halt
#endif
}

/// <summary>
/// Adds dummy data to SensorData object lists.
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
	windSpeed.addDummyGustData_10_min(25, 2, 12, 1765412100);
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

	//#if defined(VM_DEBUG)
		////////  TESTING   ////////
	if (_isDEBUG_addDummyData) {
		addDummyData();
	}

	/*if (_isDEBUG_addDummyReadings) {
		float dumVal = 0;
		unsigned int seconds = BASE_PERIOD_SEC * SECONDS_PER_HOUR * 5;
		dumVal = dummy_T.risingVal(5, 90. / seconds);
		d_Temp_F.addReading(now(), dumVal);

		d_IRSky_C.addReading(now(), dummy_IR.risingVal(-25, 90 / (seconds * 5)));

	}*/


	if (_isDEBUG_Test_setup) {
		addTestCodeHere();
	}
	//#endif

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
	sd.logStatus("SETUP END", millis());
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
		readSensors(_isDEBUG_addDummyReadings);
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




		//// LOG EXTREMA -- NEED STRING LISTS!
		d_Temp_F.maxima_daily(); // XXX Not STRING!
		d_Temp_F.minima_daily();



	}
	else {
		// SAME DAY.
		// Update summary of today's data.





	}

	/// ==========  CHECK FOR LOST WIFI CONNECTION  ========== //
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
