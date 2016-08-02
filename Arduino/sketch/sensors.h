#ifndef HEADER_SENSORS
#define HEADER_SENSORS

#include "config.h"
#include "reply.h"
#include <ArduinoJson.h>
#ifndef UDOONeo
#include <dht.h>
#else
#warning "FIX ME!"
#endif
#include <SI7021.h>
#include "TSL2561.h"
#include "Adafruit_TCS34725.h"

void handleSensorRequest(JsonObject& root);

#endif
