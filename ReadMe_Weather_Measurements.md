# Meaningful Weather Measurements

## *Random notes. Need to edit and rewrite.*

// Ralph Paonessa. 8/27/2020

Hardware interrupt to count anemometer rotations.
 
Timer interrupt to control time for rotation count.

## Goals

//    1. Integrate "instantaneous" wind speed over a specified time of
//			rotations. E.g., 2.5 sec.
//    2. Calculate average wind speed over a specified time. E.g., 10 min.
//    3. Determine wind gusts and variance during the wind averaging time.
//    4. Log or report all sensor data after a specified number of wind 
//			speed measurements.

### Standard to follow
    - measure raw windSpeed over BASE_PERIOD 2.5 sec. (This will not be 
			reported.)
    - Get maxWindSpeed as max of windSpeed over 10 min.
    - Get avgWindSpeed over 1 min.
    - Get maxWindSpeed over 1 min.
    - report speed as moving average of avgWindSpeed over a reporting 
			time of 10 min.
    - report wind gust as max wind speed during current reporting time.


=================   SENSORS and MODULES   ========================

READING				DEVICE				INTERFACE		CLASS
================	================	============	==========
Temperature			Dallas DS18B20      One-Wire		SensorData
Pressure, RH		Bosch BME280        I2C (1)			SensorData
UV A/B, Index		Vishay VEML6075     I2C (2)			SensorData
IR sky temp.		Melexis MLX90614    I2C (3)			SensorData
Wind speed			Davis Anemometer    HW interrupt	WindSpeed
Wind direction		Davis Wind Vane     ADC voltage		WindDirection
Solar radiation		PV cell             ADC voltage		SensorData
GPS data			u-blox NEO-6M       UART2			GPSModule
SD card module		generic             SPI				SDCard




 NOTE: isGetReadings (flag set by wind timer) triggers:
	1. Calculate "instantaneous" wind speed from current
		anem_RotationCount and reset anemometer rotation
		counter.
	2. Calculate fan speed and reset fan rotation counter.
	3. Reset isGetReadings (which will change at the next
		anemometer timer interrupt).
	4. Update the wind speed moving average.

*FOR SIMPLICITY, ALL THE OTHER SENSOR READINGS ARE LOGGED
 AT THIS TIME.

 NOTE: Using isGetReadings for BOTH anemometer counts and
 fan counts. This ignores the flag isGetFanRPM and the fan
 hardware timer. (This is done to avoid possibility of
 counting over different times for each. ??? )

WindSpeed wind object gives access to
	- Current speed avg for
		- 2min
		- 10 min
		- 60 min
	- Current gusts for
		- 10 min
		- 60 min
	- speed lists at 2-, 10-, and 60-min intervals
	- gust lists at 10- and 60-min intervals

	HOW DO WE WANT TO USE THIS DATA???

	1. Logging data to SD card at defined intervals. (Every 10 min??)
	2. Current readings for web display on demand.
	3. Providing trend reports over time on demand.

	Ver 11.8 implementation definined globabl variables for
	all readings, and updated these every 60 sec. These values
	were LOGGED TO SD and SERVED BY WEB SERVER.

	A 10-min running average was also tallied and logged, served.

	TIMER INTERRUPT every 2.5 sec triggers raw reading.
	2-MINUTE AVERAGE calculated every 120 sec.
	10-MINUTE AVERAGE calculated every 600 sec.

PLAN FOR READING AND REPORTING ALL SENSORS

DATA READINGS			2-min	10-min	60-min
----------------------	------	------	------
1. Wind speed & gusts	x		x		x
2. Wind direction				x		x
3. Temperature					x		x
4. Pressure						x		x
5. %RH							x		x
6. Insolation					x		x
7. UV-A							x		x
8. UV-B							x		x
9. UV-Index						x		x
10. IR sky						x		x
11. Fan speed					x		x
