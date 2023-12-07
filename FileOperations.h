/*
FileOperations.h

Exposes file operations for FS.h. These are
used with both SD cards and LittleFS.
*/

#ifndef _FILEOPERATIONS_h
#define _FILEOPERATIONS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

//#include "FS.h"
#include <FS.h>
#include <LittleFS.h>
#include "App_Settings.h"

using namespace App_Settings;

/// <summary>
/// Exposes methods that operate on files.
/// </summary>
namespace FileOperations {

	/// <summary>
	/// Lists contents of directory.
	/// </summary>
	/// <param name="fs">File system.</param>
	/// <param name="dirname">Path of the directory.</param>
	/// <param name="levels">Number of levels to list.</param>
	void dirList(fs::FS& fs, const char* dirname, uint8_t levels);

	void dirCreate(fs::FS& fs, const char* path);

	void dirRemove(fs::FS& fs, const char* path);

	/// <summary>
	/// Returns a String read from a file.
	/// </summary>
	/// <param name="fs">File system to use.</param>
	/// <param name="path">Target file path with name.</param>
	/// <returns>String read from a file.</returns>
	String fileReadString(fs::FS& fs, const char* path);

	// DOESN'T WORK!!! NOT SURE ABOUT POINTERS, BUT IS ARRAY DESTROYED WHEN FUNCTION FINISHES?!
	//char* fileReadChars(fs::FS& fs, const char* path);

	// XXX RETURNS BLANK STRING!
	/*/// <summary>
	/// Returns a String read from a file.
	/// </summary>
	/// <param name="path">Target file path with name.</param>
	/// <returns>String read from a file.</returns>
	String fileReadStringStream(const char* path);*/

	//void fileRead(fs::FS& fs, const char* path);

	void fileRename(fs::FS& fs, const char* path1, const char* path2);

	void fileDelete(fs::FS& fs, const char* path);

	/// <summary>
	/// Creates a file if it doesn't exist. Returns true on 
	/// success or if the file already exists; otherwise
	/// returns false.
	/// </summary>
	/// <param name="fs">File system to use.</param>
	/// <param name="path">
	/// Full path including the filename and extension.</param>
	/// <returns>True on success</returns>
	bool fileCreateOrExists(FS& fsys, const String& path);

	/// <summary>
	/// Writes to a file, overwriting existing data.
	/// </summary>
	/// <param name="fs">File system to use.</param>
	/// <param name="path">Target file path with name.</param>
	/// <param name="message">String to write.</param>
	void fileWrite(fs::FS& fs, const char* path, const char* message);

	/// <summary>
	/// Appends data to a file.
	/// </summary>
	/// <param name="fs">File system to use.</param>
	/// <param name="path">Target file path with name.</param>
	/// <param name="message">String to write.</param>
	void fileAppend(fs::FS& fs, const char* path, const char* message);

}
#endif