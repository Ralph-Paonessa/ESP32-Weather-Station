// Adapted from SD_Test.ino in the Arduino/ESP32 example sketches.
// 
// 

#include "FileOperations.h"

/// <summary>
/// Lists contents of directory.
/// </summary>
/// <param name="fs">File system.</param>
/// <param name="dirname">Path of the directory.</param>
/// <param name="levels">Number of levels to list.</param>
void FileOperations::dirList(fs::FS& fs, const char* dirname, uint8_t levels) {
	Serial.printf("Listing directory: %s\n", dirname);

	File root = fs.open(dirname);
	if (!root) {
		Serial.println("Failed to open directory");
		return;
	}
	if (!root.isDirectory()) {
		Serial.println("Not a directory");
		return;
	}
	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			Serial.print("  DIR : ");
			Serial.println(file.name());
			if (levels) {
				dirList(fs, file.path(), levels - 1);
			}
		}
		else {
			Serial.print("  FILE: ");
			Serial.print(file.name());
			Serial.print("  SIZE: ");
			Serial.println(file.size());
		}
		file = root.openNextFile();
	}
}

void FileOperations::dirCreate(fs::FS& fs, const char* path) {
	Serial.printf("Creating Dir: %s\n", path);
	if (fs.mkdir(path)) {
		Serial.println("Dir created.");
	}
	else {
		Serial.println("mkdir failed.");
	}
}

void FileOperations::dirRemove(fs::FS& fs, const char* path) {
	Serial.printf("Removing Dir: %s\n", path);
	if (fs.rmdir(path)) {
		Serial.println("Dir removed.");
	}
	else {
		Serial.println("dirRemove failed.");
	}
}

/// <summary>
/// Returns the contents of a file.
/// </summary>
/// <param name="fs">File system to use.</param>
/// <param name="path">Target file path, such as "/readings/data.txt".
/// </param>
/// <returns>String containing file contents.</returns>
String FileOperations::fileRead(fs::FS& fs, const char* path) {

	File file = fs.open(path, FILE_READ);
	if (!file) {
		Serial.println("ERROR: fileRead failed to open file for reading");
		return "";
	}
	char stringC[DATA_FILE_BUFFER_SIZE] = {  };	// C-string array to hold values from stream.
	int i = 0;
	while (file.available()) {
		stringC[i] += file.read();	// Add each character to C-string.
		i++;
	}
	file.close();
	return String(stringC);
}


void FileOperations::fileWrite(fs::FS& fs, const char* path, const char* message) {
	Serial.printf("Writing file: %s\n", path);

	File file = fs.open(path, FILE_WRITE);
	if (!file) {
		Serial.println("ERROR: fileWrite Failed to open file for writing");
		return;
	}
	if (file.print(message)) {
		//Serial.println("fileWrite File written.");
	}
	else {
		Serial.println("ERROR: fileWrite Write failed.");
	}
	file.close();
}


void FileOperations::fileAppend(fs::FS& fs, const char* path, const char* message) {
	Serial.printf("fileAppend Appending to file: %s\n", path);
	File file = fs.open(path, FILE_APPEND);
	if (!file) {
		Serial.println("ERROR: fileAppend Failed to open file for appending.");
		return;
	}
	if (file.print(message)) {
		Serial.println("fileAppend Message appended.");
	}
	else {
		Serial.println("ERROR: fileAppend Append failed.");
	}
	file.close();
}

void FileOperations::fileRename(fs::FS& fs, const char* path1, const char* path2) {
	Serial.printf("Renaming file %s to %s.\n", path1, path2);
	if (fs.rename(path1, path2)) {
		Serial.println("File renamed.");
	}
	else {
		Serial.println("ERROR: Rename failed.");
	}
}

void FileOperations::fileDelete(fs::FS& fs, const char* path) {
	Serial.printf("Deleting file: %s\n", path);
	if (fs.remove(path)) {
		Serial.println("File deleted.");
	}
	else {
		Serial.println("ERROR: Delete failed.");
	}
}

/// <summary>
/// Creates a file if it doesn't exist. Returns true on 
/// success or if the file already exists; otherwise
/// returns false. XXX IS THIS NEEDED?!
/// </summary>
/// <param name="fs">File system to use.</param>
/// <param name="path">Full path including the filename and extension.</param>
/// <returns>True on success</returns>
bool FileOperations::fileCreateOrExists(fs::FS& fs, const String& path) {
	//if (true)            //(!_isBypassSDCard)
	//{
		// If the file doesn't exist, create it.
	if (fs.exists(path)) {
		Serial.printf("File %s exists.\n", path.c_str());
		return true;
	}
	else {
		Serial.printf("File %s does not exist.\n", path.c_str());
	}

	// File does not exist, so create empty file.
	File file = fs.open(path, FILE_WRITE);
	if (!file) {
		Serial.printf("File %s could not be opened.\n", path.c_str());
		return false;
	}
	else {
		Serial.printf("File %s was opened and will be closed.\n", path.c_str());
		file.close();
		return true;
	}
	//}
}
