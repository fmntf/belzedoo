#include "servo.h"

extern char jsonOut[256];
extern int written;

Servo servos[53];

void handleServoRequest(JsonObject& root)
{
  const char* method = root["servo"];
  int pin = root["pin"];
  StaticJsonBuffer<200> responseJsonBuffer;
  JsonObject& response = responseJsonBuffer.createObject();
  response["id"] = root["id"];

  if (strcmp(method, "attach") == 0) {
    servos[pin].attach(pin);
  } else if (strcmp(method, "detach") == 0) {
    servos[pin].detach();
  } else if (strcmp(method, "write") == 0) {
    int degrees = root["degrees"];
    servos[pin].write(degrees);
  } else {
    response["success"] = (bool)false;
  }
  
  response["success"] = (bool)true;

  written = response.printTo(jsonOut, 255);
  jsonOut[written] = '\n';
  written++;
  jsonOut[written] = '\0';
  reply(jsonOut, written);
}
