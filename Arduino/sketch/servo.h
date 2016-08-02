#ifndef HEADER_SERVO
#define HEADER_SERVO

#include "config.h"
#include "reply.h"
#include <ArduinoJson.h>
#include <Servo.h>

void handleServoRequest(JsonObject& root);

#endif
