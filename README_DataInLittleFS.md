# Saving Volatile Data in LittleFS File System

If the sensor readings at various intervals (10-min, 60-min, dailyMax, 
dailyMin) are only kept in RAM, they will be lost when a reboot is 
required. Instead, we will store them in the LittleFS flash file system
and retrieve them from there when a user requests data.

## File structure
Text files will be saved for each sensor and named using SensorData.labelFile().
The data will be saved in the same delimited text format that is served
by the web server as a single line of data:

    t1,val1\~t2,val2\~t2,val2 ...

Data will be saved for intervals of
 - 10 minutes
 - 60 minutes
 - daily (with max and min combined, delimited by "|")

## FileOperations custom library
This library exposes methods to work with the files in a filesystem, and 
can work with both SD and LittleFS.

Files can be written to LittleFS by simply using

    fileWrite(fs::FS& fs, const char* path, const char* message)

Retrieving the data for the web server charts is a little more problematic. 
Data previously has been served using a function that returns a String:

    String FileOperations::data_10_min_string_delim()

This must be converted to a character array using c_str().

    request->send_P(200, "text/plain", d_Insol.data_10_min_string_delim().c_str());

At present, I've created a new function to read that data file and return
the results as a String.

    String FileOperations::fileReadString(fs::FS& fs, const char* path)

This seems *inefficient* because fileReadString operates by reading the file 
characters into a char[] array, and then converting it to String using
String(array). But it must then be *converted back* to a char[] array for 
request->send_P(). But I don't see a preferable approach. In practice, this method 
may not put much of a burdern on the processor.

## When to retrieve data from flash
As long as there is room in memory, I will continue to keep the readings history 
in lists in memory rather than take the extra time to read them from flash when 
needed for the web server.

Instead, a routine is needed to retrieve them from flash and put the lists back 
in memory if the system has to reboot. It is necessary to identify which data 
is sufficiently fresh to reuse after a boot. This is straightforward because the
data includes the reading times.

## Which data to retrieve
It is more important to retrieve the daily values, even after an outage of 1 
day or more. Next in priority is the hourly data, although this will damage the 
daily values if it is too old.

The 10-min values can probably be ignored, as they are most likely to have a time
gap and are the quickest to reestablish with new data.

This can be handled by having different recovery time thresholds for each data 
period:
 - 10-min - <30 minutes
 - 60-min - <3 hours
 - daily max and min - <3 days