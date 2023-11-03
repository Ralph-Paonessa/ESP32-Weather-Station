// 
// 
// 

#include "FileOperations.h"
void FileOperations::listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
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
                listDir(fs, file.path(), levels - 1);
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

void FileOperations::createDir(fs::FS& fs, const char* path) {
    Serial.printf("Creating Dir: %s\n", path);
    if (fs.mkdir(path)) {
        Serial.println("Dir created");
    }
    else {
        Serial.println("mkdir failed");
    }
}

void FileOperations::removeDir(fs::FS& fs, const char* path) {
    Serial.printf("Removing Dir: %s\n", path);
    if (fs.rmdir(path)) {
        Serial.println("Dir removed");
    }
    else {
        Serial.println("rmdir failed");
    }
}

void FileOperations::readFile(fs::FS& fs, const char* path) {
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}

void FileOperations::writeFile(fs::FS& fs, const char* path, const char* message) {
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        Serial.println("File written");
    }
    else {
        Serial.println("Write failed");
    }
    file.close();
}

void FileOperations::appendFile(fs::FS& fs, const char* path, const char* message) {
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        Serial.println("Message appended");
    }
    else {
        Serial.println("Append failed");
    }
    file.close();
}

void FileOperations::renameFile(fs::FS& fs, const char* path1, const char* path2) {
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    }
    else {
        Serial.println("Rename failed");
    }
}

void FileOperations::deleteFile(fs::FS& fs, const char* path) {
    Serial.printf("Deleting file: %s\n", path);
    if (fs.remove(path)) {
        Serial.println("File deleted");
    }
    else {
        Serial.println("Delete failed");
    }
}





/// <summary>
/// Creates a file if it doesn't exist. Returns true on 
/// success or if the file already exists; otherwise
/// returns false.
/// </summary>
/// <param name="fs">File system to use.</param>
/// <param name="path">Full path including the filename and extension.</param>
/// <returns>True on success</returns>
bool FileOperations::createFile(fs::FS& fs, const String& path) {
	if (true)            //(!_isBypassSDCard)
	{
		// If the file doesn't exist, create it.
		if (fs.exists(path)) {
			//String msg = "[SDCard.createFile] " + path + " file found.";
			//logStatus(msg, millis());
			return true;
		}
		// File does not exist, so create empty file.
		File file = fs.open(path, FILE_WRITE);
		if (!file) {
			//String msg = "[SDCard.createFile] " + path + " file could not be created.";
			//logStatus(msg, millis());
			return false;
		}
		else {
			//String msg = "[SDCard.createFile] " + path + " file created.";
			//logStatus(msg, millis());
			file.close();
			return true;
		}
	}
}

///// <summary>
///// Writes to an file, overwriting existing data.
///// </summary>
///// <param name="fs">File system to use.</param>
///// <param name="path">Target file path with name.</param>
///// <param name="msg">String to write.</param>
//void FileOperations::writeFile(fs::FS& fs, const char* path, const char* msg) {
//	Serial.printf("Writing file: %s\n", path);
//	File file = fs.open(path, FILE_WRITE);
//	if (!file) {
//		Serial.println("ERROR: writeFile failed to open file for writing");
//		return;
//	}
//	if (file.print(msg)) {
//		Serial.println("File written");
//	}
//	else {
//		Serial.println("ERROR: writeFile failed");
//	}
//	file.close();
//}

///// <summary>
///// Appends to a file.
///// </summary>
///// <param name="fs">File system to use.</param>
///// <param name="path">Target file path with name.</param>
///// <param name="msg">String to write.</param>
//void FileOperations::appendFile(fs::FS& fs, const char* path, const char* msg) {
//
//	File file = fs.open(path, FILE_APPEND);
//	if (!file) {
//		String s = "ERROR: appendFile failed to open """;
//		s += String(path) + """ for appending";
//		Serial.println(s);
//		return;
//	}
//	if (!file.print(msg)) {
//		Serial.println("ERROR: appendFile failed for :");
//		Serial.print("--|"); Serial.print(msg); Serial.println("|--");
//	}
//	file.close();
//}
