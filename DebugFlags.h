// DebugFlags.h

#ifndef _DEBUGFLAGS_h
#define _DEBUGFLAGS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


/**************************************************************/
/*****************      DEBUGGING FLAGS      ******************/
/**************************************************************/

bool _isDEBUG_BypassGPS = true;			// Bypass gps syncing.
bool _isDEBUG_BypassWifi = false;		// Bypass WiFi connect.
bool _isDEBUG_BypassSDCard = true;		// Bypass SD card.
bool _isDEBUG_ListLittleFS = false;		// List contents of LittleFS.
bool _isDEBUG_BypassWebServer = false;	// Bypass Web Server.
bool _isDEBUG_Test_setup = false;		// Run only test code inserted in Setup.
bool _isDEBUG_Test_loop = false;		// Run test code inserted in Loop.
bool _isDEBUG_addDummyData = false;		// Add dummy data.
bool _isDEBUG_simulateReadings = true;	// Add dummy sensor reading values.
bool _isDEBUG_AddDelayInLoop = false;	// Add delay in loop.
const int _LOOP_DELAY_DEBUG_ms = 100;	// Debug delay in loop, msec.
/************************************************************/


#endif
