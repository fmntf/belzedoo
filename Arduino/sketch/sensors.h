#ifndef HEADER_SENSORS
#define HEADER_SENSORS

#include "config.h"
#include "reply.h"

#include <ArduinoJson.h>
#include <dht.h>
#include <Adafruit_TCS34725.h>

#ifdef HAS_BRICK_SUPPORT
#include <SI7021.h> // Humidity Brick
#include <LM75.h> // Temperature Brick
#include <TSL2561.h> // Light Brick
#ifdef ARDUINO_UDOO_NEO
#include <Adafruit_MPL3115A2_neo_pragma.h> // Barometer Brick
#else
#include <Adafruit_MPL3115A2.h> // Barometer Brick
#endif
#endif //HAS_BRICK_SUPPORT

void handleSensorRequest(JsonObject& root);

#endif
