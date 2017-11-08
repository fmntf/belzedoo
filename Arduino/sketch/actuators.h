#ifndef HEADER_ACTUATORS
#define HEADER_ACTUATORS

#include "config.h"
#include "reply.h"
#include <ArduinoJson.h>
#include <ChainableLED.h>

void handleActuatorRequest(JsonObject& root);

#endif
