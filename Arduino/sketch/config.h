#ifndef HEADER_CONFIG
#define HEADER_CONFIG

#include "Arduino.h"

#define SKETCH_VERSION 4
//#define ENABLE_SERIAL_DEBUG

// end of configuration!

#ifdef ARDUINO_UDOO_NEO
  #ifdef ENABLE_SERIAL_DEBUG
    #define SERIAL_DEBUG Serial0
  #endif
#else
  #ifdef __ARDUINO_ARC__
    #ifdef ENABLE_SERIAL_DEBUG
      #define SERIAL_DEBUG Serial1
    #endif
  #else
    #ifdef ENABLE_SERIAL_DEBUG
      #define SERIAL_DEBUG Serial1
    #endif
    #define HAS_ADK
  #endif
#endif

#endif
