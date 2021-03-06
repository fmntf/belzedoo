#ifndef HEADER_CONFIG
#define HEADER_CONFIG

#include "Arduino.h"

#define SKETCH_VERSION 6
//#define ENABLE_SERIAL_DEBUG
#ifdef ENABLE_SERIAL_DEBUG
//#define VERBOSE_DEBUG
#endif
// end of configuration!

#ifdef ARDUINO_UDOO_NEO
  // UDOO Neo
  #define HAS_BRICK_SUPPORT
  #ifdef ENABLE_SERIAL_DEBUG
    #define SERIAL_DEBUG Serial0
  #endif
  #define MAX_PINS 13
#else
  #ifdef __ARDUINO_ARC__
    // UDOO X86
    #define HAS_BRICK_SUPPORT
    #ifdef ENABLE_SERIAL_DEBUG
      #define SERIAL_DEBUG Serial1
    #endif
    #define MAX_PINS 13
    #define ARDUINO_UDOO_X86
  #else
    // UDOO Quad
    #ifdef ENABLE_SERIAL_DEBUG
      #define SERIAL_DEBUG Serial1
    #endif
    #define HAS_ADK
    #define MAX_PINS 53
    #define ARDUINO_UDOO_QDL
  #endif
#endif

#endif
