#ifndef HEADER_CONFIG
#define HEADER_CONFIG

#include "Arduino.h"

#define SKETCH_VERSION 5
#define ENABLE_SERIAL_DEBUG

// end of configuration!

#ifdef ARDUINO_UDOO_NEO
  #define HAS_BRICK_SUPPORT
  #ifdef ENABLE_SERIAL_DEBUG
    #define SERIAL_DEBUG Serial0
  #endif
  #define MAX_PINS 13
#else
  #ifdef __ARDUINO_ARC__
    #ifdef ENABLE_SERIAL_DEBUG
      #define SERIAL_DEBUG Serial1
    #endif
    #define MAX_PINS 13
  #else
    #ifdef ENABLE_SERIAL_DEBUG
      #define SERIAL_DEBUG Serial1
    #endif
    #define HAS_ADK
    #define MAX_PINS 53
  #endif
#endif

#endif
