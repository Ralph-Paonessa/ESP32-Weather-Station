# v12.0.1 active

# ESP32-Weather-Station

Contains code and libraries to get, process, and display weather sensor readings from an ESP32 development board.

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
- Async web server that serves dynamic web pages from ESP32 flash memory (LittleFS) and SD card

## Data reporting
### Web server
By connecting to the same WiFi network as the station, the following data is available from the ESP32 web server:
- Current ("instantaneous") readings
- Charts of readings at 10-minute intervals
- Charts of readings at 60-minute intervals
### SD card
- Data.txt file with readings at 10-minute intervals.
- Log.txt file with status messages to monitor performance and for debugging.
 
Currently, the 10- and 60-minutes readings are stored only in volatlie memory and are lost when the ESP32 is restarted.

Future plans include storing this data in flash memory (LittleFS) so that it persists upon restart.

## Weather Station
The weather station is a DIY custom design incorporating various commerically-available sensors and a custom DIY rdiation shield for the temperature sensor.

## Development
It is through this project that I have started to learn to write C++ code and interface with the ESP-32 through the ESP32-Arduino IDE.

In that regard, I have benefitted from numerous online tutorials on the ESP32 and using it with numerous sensors. I have attempted to go beyond simple implementations and design a system that can collect and store various weather data for my (idiosyncratic) needs.