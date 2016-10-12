#ifndef HEADER_CONFIG
#define HEADER_CONFIG

#include "Arduino.h"

#define SKETCH_VERSION 3

#ifdef ARDUINO_UDOO_NEO
#define SERIAL_DEBUG Serial0
#else
#define SERIAL_DEBUG Serial1
#define HAS_ADK
#endif

#endif
