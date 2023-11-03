# Saving Volatile Data in LittleFS File System

If the sensor readings at various intervals (10-min, 60-min, dailyMax, 
dailyMin) are only kept in RAM, they will be lost when a reboot is 
required. Instead, we will store them in the LittleFS flash file system
and retrieve them from there when a user requests data.

## File structure
Text files will be saved for each sensor and named using SensorData.name().
The data will be saved in the same delimited text format that is served
by the web server:

t1,val1\~t2,val2\~t2,val2 ...