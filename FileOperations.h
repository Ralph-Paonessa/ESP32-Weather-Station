/*
FileOperations.h

Exposes file operations for FS.h. These are
used with both the SD card and LittleFS.
*/


#ifndef _FILEOPERATIONS_h
#define _FILEOPERATIONS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "FS.h"
#include <LittleFS.h>

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
	void listDir(fs::FS& fs, const char* dirname, uint8_t levels);

	void createDir(fs::FS& fs, const char* path); 

	void removeDir(fs::FS& fs, const char* path);

	String readFile_intoString(fs::FS& fs, const char* path, const char terminator = '\n');

	void readFile(fs::FS& fs, const char* path);

	void writeFile(fs::FS& fs, const char* path, const char* message); 

	void appendFile(fs::FS& fs, const char* path, const char* message); 

	void renameFile(fs::FS& fs, const char* path1, const char* path2);

	void deleteFile(fs::FS& fs, const char* path);




	/// <summary>
	/// Creates a file if it doesn't exist. Returns true on 
	/// success or if the file already exists; otherwise
	/// returns false.
	/// </summary>
	/// <param name="fs">File system to use.</param>
	/// <param name="path">
	/// Full path including the filename and extension.</param>
	/// <returns>True on success</returns>
	bool create_or_existsFile(FS& fsys, const String& path);

	/// <summary>
	/// Writes to a file, overwriting existing data.
	/// </summary>
	/// <param name="fs">File system to use.</param>
	/// <param name="path">Target file path with name.</param>
	/// <param name="message">String to write.</param>
	void writeFile(fs::FS& fs, const char* path, const char* message);

	/// <summary>
	/// Appends data to a file.
	/// </summary>
	/// <param name="fs">File system to use.</param>
	/// <param name="path">Target file path with name.</param>
	/// <param name="message">String to write.</param>
	void appendFile(fs::FS& fs, const char* path, const char* message);






}

#endif

