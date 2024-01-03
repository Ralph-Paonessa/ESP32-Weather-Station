// PinAssignments.h

#ifndef _PINASSIGNMENTS_h
#define _PINASSIGNMENTS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

const int SPI_CS_PIN = 5;	// CS pin for the SD card module
const int RX2_PIN = 16;                 // UART2 U2_RXD
const int TX2_PIN = 17;                 // UART2 U2_RXD
const int FAN_PWM_PIN = 27;				//  GPIO 27 - fan PWM control pin
const int FAN_SPEED_PIN = 35;			// Fan speed pin GPIO 35 (Digital Input with pullup).
const int WIND_SPEED_PIN = 15;					// GPIO 15 (Digital Input with pullup)
const int ONE_WIRE_PIN = 4;   // GPIO 4 pin for DS18B20
const int WIND_VANE_PIN = 34;		// Wind vane is connected to GPIO 34 (Analog ADC1 CH6)
const int SOLAR_PIN = 32;	// Photovoltaic cell uses GPIO 32 (Analog ADC1 CH4)



#endif

