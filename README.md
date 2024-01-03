# v1.0.1 active

# ESP32-Weather-Station

Contains code and libraries to receive, process, and display weather 
sensor readings from an ESP32 development board.

## Implemented sensors
- Ambient temperature
- Relative humidity
- Wind speed and wind gusts
- Wind direction
- Atmospheric pressure
- Insolation (relative Sun intensity)
- UV Index
- Infrared sky temperature 

## Implemented devices and modules
- GPS module (for time and elevation)
- SD card (for logging data and status messages)
- PWM control of fan for airflow in radiation shield
- WiFi client
- Async web server that serves dynamic web pages from ESP32 flash 
- memory (LittleFS) and SD card

## Data reporting
### Web server
By connecting to the same WiFi network as the station, the following 
data is available from the ESP32 web server:
- Current ("instantaneous") readings
- Charts of readings at 10-minute intervals
- Charts of readings at 60-minute intervals
### SD card
- Data.txt file with readings at 10-minute intervals.
- Log.txt file with status messages to monitor performance and for 
- debugging.
 
Currently, the 10- and 60-minutes readings are stored only in 
volatlie memory and are lost when the ESP32 is restarted.

Future plans include storing this data in flash memory (LittleFS) so 
that it persists upon restart.

## Weather Station
The weather station is a DIY custom design incorporating various 
commerically-available sensors and a custom DIY radiation shield 
for the temperature sensor. For my needs, the station must be 
portable (set up somewhere and run for a week or more) and run 
on batteries that are recharged by solar power. The power 
management is not handled in this code.

## Custom libraries and separate .ino files
I have found it convenient to create several C++ libraries, to make  
use of reusable code as well as to reduce the size of the main sketch for 
clarity.

### App_Settings.h
Various settings centralized in one location.

### SensorData.h, SensorData.cpp
Implements a SensorData class that can be used to read and 
minipulate data from a variety of sensors.

### WindSpeed.h, WindSpeed.cpp
Impements a WindSpeed class that inherits from SensorData but adds  
functionality for measuring wind gusts, which are unique to wind 
measurements. (Note: Wind gust measurements provided by WindSpeed are 
ultimately added to a SensorData object that handles that data.)

### WindDirection.h, WindDirection.cpp
Impements a WindDirection class that inherits from SensorData but adds  
functionality for measuring wind direction (in particular, the ability 
to vector-average the wind direction).

### DataPoint.h
Creates a DataPoint structure with properties time and value to hold
sensor reading times and values.

### GPSModule.h, GPSModule.cpp
Implements a GPSModule class that handles both GPS interactions and 
system timekeeping. It makes use of the libraries TinyGPSPLus by Mikal 
Hart (https://github.com/mikalhart/TinyGPSPlus) and TimeLib by Paul 
Stoffregen (https://github.com/PaulStoffregen/Time). Because the primary
function of the GPS is to determine the local time, it was useful to
combine time and GPS functions into one library.

### SDCard.h, SDCard.cpp
Implements an SDCard class with methods to read and write to an SD card. 

### ListFunctions.h, ListFunctions.cpp
Implements methods to deal with C++ std::list operations, 
which are used to hold, analyze, average, and otherwise operate on lists 
of weather station data.

### Utilities.h, Utilities.cpp
Implements methods for processing weather data.

### SensorSimulate.h, SensorSimulate.cpp
Implements methods to generate simulated reading values through time, 
for testing and debugging.

### Testing.h, Testing.cpp
Implements methods for testing and creation of dummy data.

### Breaking the main sketch into multiple .ino files
In an attempt to organize and simplify the main sketch, I have 
separated it into several files:

- Weather Station.ino: the main sketch
- WebServer.ino: routing for the asynchronous web server
- Webserver Processor.ino: the processor function for AsyncWebServer 
- that replaces placeholder text in html files with appropriate values.
- DebugFlags.h: Flags that turn on various debugging or simulation features.

## Development
It is through this project that I have started to learn to write 
C++ code and interface with the ESP-32 through the ESP32-Arduino IDE.

In this regard, 
I have benefitted from numerous online tutorials on the ESP32 
and how to use it with various sensors. I have attempted to go 
beyond simple implementations and design a system that can collect 
and store various weather data for my (idiosyncratic) needs.