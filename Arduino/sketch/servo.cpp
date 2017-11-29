#include "servo.h"

extern char jsonOut[256];
extern int written;


Servo servos[MAX_PINS];

void handleServoRequest(JsonObject& root)
{
  const char* method = root["servo"];
  int pin = root["pin"];
  StaticJsonBuffer<200> responseJsonBuffer;
  JsonObject& response = responseJsonBuffer.createObject();
  response["id"] = root["id"];

  if (strcmp(method, "attach") == 0) {
    response["deprecated"] = (bool)true;
    if (!servos[pin].attached()) {
      servos[pin].attach(pin);
    }
  } else if (strcmp(method, "detach") == 0) {
    if (servos[pin].attached()) {
      servos[pin].detach();
    }
  } else if (strcmp(method, "write") == 0) {
    int degrees = root["degrees"];

    if (!servos[pin].attached()) {
      servos[pin].attach(pin);
    }
    
    if (servos[pin].read() != degrees) {
      servos[pin].write(degrees);
    }
   
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
