// 
// 
// 

#include "SDCard.h"

/// <summary>
/// Creates SD card instance. 
/// </summary>
SDCard::SDCard() {}

/// <summary>
/// Initializes SD card module.
/// </summary>
/// <param name="SPI_CS_pin">GPIO pin number.</param>
/// <param name="isBypass">
/// Set true to bypass SD card operations and add 
/// dummy data.</param>
/// <returns>True if successful SD card initialization.</returns>
bool SDCard::initialize(int SPI_CS_pin, bool isBypass) {

	_SPI_CS_pin = SPI_CS_pin;

	_isBypassSDCard = isBypass;
	if (isBypass) {
		// Bypass initializing SD card		
		Serial.print(millis() / 1000.);
		Serial.println("s [SDCard.initialize] BYPASS SD card.");
		return false;
	}
	// Initialize SD card
	Serial.print(millis() / 1000.);
	Serial.println("s [SDCard.initialize] Initializing SD card.");
	if (SD.begin(_SPI_CS_pin)) {
		// Success.
		logStatus(); logStatus();	// empty lines
		logStatus(LINE_SEPARATOR_LOG_BEGINS);
		logStatus("[SDCard.initialize] Logging has started.", millis());
		logStatus("[SDCard.initialize] MicroSD card mount successful.", millis());
		return true;
	}
	else {
		// Failure.
		logStatus("[SDCard.initialize] ERROR: MicroSD card mount failed.", millis());
		return false;
	}
}

///// <summary>
///// Initializes SD card module.
///// </summary>
///// <param name="gps">GPSModule object (for datetime)</param>
///// <param name="isBypass">Set true to bypass SD card operations.</param>
///// <returns>True if successful SD card initialization.</returns>
//bool SDCard::initialize(bool isBypass) {
//	_isBypassSDCard = isBypass;
//	if (isBypass) {
//		// Bypass initializing SD card		
//		Serial.print(millis() / 1000.);
//		Serial.println("s [SDCard.initialize] BYPASS SD card.");
//		return false;
//	}
//	// Initialize SD card
//	Serial.print(millis() / 1000.);
//	Serial.println("s [SDCard.initialize] Initializing SD card.");
//	if (SD.begin(_SPI_CS_pin)) {
//		// Success.
//		logStatus(); logStatus();	// empty lines
//		logStatus(LINE_SEPARATOR_LOG_BEGINS);
//		logStatus("[SDCard.initialize] Logging has started.", millis());
//		logStatus("[SDCard.initialize] MicroSD card mount successful.", millis());
//		return true;
//	}
//	else {
//		// Failure.
//		logStatus("[SDCard.initialize] ERROR: MicroSD card mount failed.", millis());
//		return false;
//	}
//}

void SDCard::setBypass()
{
	_isBypassSDCard = true;
}

void SDCard::cancelBypass()
{
	_isBypassSDCard = false;
}

/// <summary>
/// Writes to an SD card file, overwriting existing data.
/// </summary>
/// <param name="fs">File system object.</param>
/// <param name="path">Target file path with name.</param>
/// <param name="msg">String to write.</param>
void SDCard::writeFile(fs::FS& fs, const char* path, const char* msg) {
	Serial.printf("Writing file: %s\n", path);
	File file = fs.open(path, FILE_WRITE);
	if (!file) {
		Serial.println("ERROR: writeFile failed to open file for writing");
		return;
	}
	if (file.print(msg)) {
		Serial.println("File written");
	}
	else {
		Serial.println("ERROR: writeFile failed");
	}
	file.close();
}

/// <summary>
/// Appends to an SD card file.
/// </summary>
/// <param name="fs">File system object.</param>
/// <param name="path">Target file path with name.</param>
/// <param name="msg">String to write.</param>
void SDCard::appendFile(fs::FS& fs, const char* path, const char* msg) {

	File file = fs.open(path, FILE_APPEND);
	if (!file) {
		String s = "ERROR: appendFile failed to open """;
		s += String(path) + """ for appending";
		Serial.println(s);
		return;
	}
	if (!file.print(msg)) {
		Serial.println("ERROR: appendFile failed for :");
		Serial.print("--|"); Serial.print(msg); Serial.println("|--");
	}
	file.close();
}

/// <summary>
/// Appends a line of data to the DATA file.
/// </summary>
/// <param name="msg">String to write.</param>
void SDCard::logData(String msg) {
#if defined(VM_DEBUG)
	Serial.println(msg);     // Echo to serial monitor
#endif
	if (!_isBypassSDCard) {
		String status = msg + "\r\n";	// Append CR + LF.
		appendFile(SD, LOGFILE_PATH_DATA.c_str(), status.c_str());
	}
}

/// <summary>
/// Writes indented line to status log 
/// (and serial monitor if VM_DEBUG).
/// </summary>
/// <param name="msg">Message to log.</param>
void SDCard::logStatus_indent(const String& msg) {
#if defined(VM_DEBUG)
	Serial.println("\t" + msg);     // Echo to serial monitor
#endif
	if (!_isBypassSDCard) {
		String status = "\t" + msg + "\r\n";	// Append CR + LF.
		appendFile(SD, LOGFILE_PATH_STATUS.c_str(), status.c_str());
	}
}

/// <summary>
/// Writes blank line to status log (and serial monitor if VM_DEBUG).
/// </summary>
void SDCard::logStatus() {
#if defined(VM_DEBUG)
	Serial.println();	// Echo to serial monitor
#endif
	if (!_isBypassSDCard) {
		appendFile(SD, LOGFILE_PATH_STATUS.c_str(), "\r\n");
	}
}

/// <summary>
/// Writes line to status log (and serial monitor if VM_DEBUG).
/// </summary>
/// <param name="msg">Message to log.</param>
void SDCard::logStatus(const String& msg) {
#if defined(VM_DEBUG)
	Serial.println(msg);	// Echo to serial monitor
#endif
	if (!_isBypassSDCard) {
		String status = msg + "\r\n";	// Append CR + LF.
		appendFile(SD, LOGFILE_PATH_STATUS.c_str(), status.c_str());
	}
}

/// <summary>
/// Writes line prefixed by date to status 
/// log (and serial monitor if VM_DEBUG).
/// </summary>
/// <param name="msg">Message to log.</param>
/// <param name="dateString">Date and time string to include.</param>
void SDCard::logStatus(const String& msg, const String& dateString) {
	String status = dateString + " ";
	status += msg;
#if defined(VM_DEBUG)
	Serial.println(status);	// Echo to serial monitor
#endif
	if (!_isBypassSDCard) {
		status += "\r\n";	// Append CR + LF.
		appendFile(SD, LOGFILE_PATH_STATUS.c_str(), status.c_str());
	}
}

/// <summary>
/// Writes message line, prefixed by seconds since start,
/// to status log (and serial monitor if VM_DEBUG).
/// </summary>
/// <param name="msg">Message to log.</param>
/// <param name="millisec">Milliseconds since start).</param>
void SDCard::logStatus(const String& msg, unsigned long millisec) {
	String status = String(millisec / 1000., 2) + "s ";
	status += msg;
#if defined(VM_DEBUG)
	Serial.println(status);	// Echo to serial monitor
#endif
	if (!_isBypassSDCard) {
		status += "\r\n";	// Append CR + LF.
		appendFile(SD, LOGFILE_PATH_STATUS.c_str(), status.c_str());
	}
}

// Create a file on the SD card, if it doesn't
// already exist. Returns true on success
// or if the file already exists; otherwise
// returns false.
bool SDCard::createFile(const String& path) {
	if (!_isBypassSDCard)
	{
		// If the file doesn't exist, create it.
		if (SD.exists(path)) {
			String msg = "[SDCard.createFile] " + path + " file found.";
			logStatus(msg, millis());
			return true;
		}
		// File does not exist, so create empty file.
		File file = SD.open(path, FILE_WRITE);
		if (!file) {
			String msg = "[SDCard.createFile] " + path + " file could not be created.";
			logStatus(msg, millis());
			return false;
			return false;
		}
		else {
			String msg = "[SDCard.createFile] " + path + " file created.";
			logStatus(msg, millis());
			file.close();
			return true;
		}
	}
}
