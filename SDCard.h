// SDCard.h

#ifndef _SDCARD_h
#define _SDCARD_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>

#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>

#include <SPI.h>

#include "App_Settings.h"
using namespace App_Settings;

/// <summary>
/// Exposes methods to read/write files to an SD card.
/// </summary>
class SDCard {

private:
	int _SPI_CS_pin;	// SPI pin number for SD card.
	bool _isBypassSDCard = false;

	/// <summary>
	/// Writes to the SD card file, overwriting existing data.
	/// </summary>
	/// <param name="fs">File system object.</param>
	/// <param name="path">Target file path with name.</param>
	/// <param name="message">String to write.</param>
	void writeFile(fs::FS& fs, const char* path, const char* message);
	
	/// <summary>
	/// Appends data to an SD card file.
	/// </summary>
	/// <param name="fs">File system object.</param>
	/// <param name="path">Target file path with name.</param>
	/// <param name="message">String to write.</param>
	void appendFile(fs::FS& fs, const char* path, const char* message);

public:
	/// <summary>
	/// Creates SD card instance. 
	/// </summary>
	SDCard();

	/// <summary>
	/// Initializes SD card module.
	/// </summary>
	/// <param name="SPI_CS_pin">GPIO pin number.</param>
	/// <param name="isBypass">Set true to bypass SD card operations.</param>
	/// <returns>True if successful SD card initialization.</returns>
	bool initialize(int SPI_CS_pin, bool isBypass);

	/// <summary>
	/// Sets SDCard module to bypass logging to the card.
	/// </summary>
	void setBypass();

	/// <summary>
	/// Sets SDCard module to not bypass logging to the card.
	/// </summary>
	void cancelBypass();

	/// <summary>
	/// Create the file if it does not exist.
	/// </summary>
	/// <param name="path">The file path including name.</param>
	/// <returns>True if file found or created.</returns>
	bool createFile(const String& path);

	/// <summary>
	/// Appends data to the SD card, and advances to new line.
	/// </summary>
	/// <param name="logEntry">String to write.</param>
	void logData(String logEntry);

	/// <summary>
	/// Writes blank line to status log (and serial monitor if VM_DEBUG).
	/// </summary>
	void logStatus();

	/// <summary>
	/// Writes line to status log (and serial monitor if VM_DEBUG).
	/// </summary>
	/// <param name="msg">Message to log.</param>
	void logStatus(const String& msg);

	/// <summary>
	/// Writes indented line to status log 
	/// (and serial monitor if VM_DEBUG).
	/// </summary>
	/// <param name="msg">Message to log.</param>
	void logStatus_indent(const String& msg);

	/// <summary>
	/// Writes line prefixed by date to status 
	/// log (and serial monitor if VM_DEBUG).
	/// </summary>
	/// <param name="msg">Message to log.</param>
	/// <param name="dateString">Date and time string to include.</param>
	void logStatus(const String& msg, const String& dateString);

	/// <summary>
	/// Writes line prefixed by seconds since start to
	/// status log (and serial monitor if VM_DEBUG).
	/// </summary>
	/// <param name="msg">Message to log.</param>
	/// <param name="millisec">Milliseconds since start).</param>
	void logStatus(const String& msg, unsigned long millisec);
};

#endif
