#include "actuators.h"

extern char jsonOut[256];
extern int written;

void handleActuatorRequest(JsonObject& root)
{
  const char* actuator = root["actuator"];
  StaticJsonBuffer<200> responseJsonBuffer;
  JsonObject& response = responseJsonBuffer.createObject();
  response["id"] = root["id"];

  if (strcmp(actuator, "P9813") == 0) { // Groove Chainable LED

    int pin = root["pin"];
    const char* mode = root["mode"];

    ChainableLED leds(pin, pin+1, 1);
    leds.init();
    response["success"] = (bool)true;

    if (strcmp(mode, "hsb") == 0) {
      int h = root["h"];
      int s = root["s"];
      int b = root["b"];
      leds.setColorHSB(0, h/255.0, s/255.0, b/255.0);
    } else if (strcmp(mode, "rgb") == 0) {
      int r = root["r"];
      int g = root["g"];
      int b = root["b"];
      leds.setColorRGB(0, r, g, b);
    } else if (strcmp(mode, "rainbow") == 0) {
      int n = root["n"];
      int d = root["delay"];
      for (int cycle=0; cycle<n; cycle++) {
        for (float hue=0.0; hue<=1.0; hue+=0.02) {
          leds.setColorHSB(0, hue, 1.0, 0.55);
          delay(d);
        }
        for (float hue=1.0; hue>=0.0; hue-=0.02) {
          leds.setColorHSB(0, hue, 1.0, 0.55);
          delay(d);
        }
        leds.setColorHSB(0, 0.0, 0.0, 0.0);
      }
    } else {
      response["success"] = (bool)false;
      response["error"] = "INVALID_MODE";      
    }
  }
  
  else {
    response["success"] = (bool)false;
    response["error"] = "NO_ACTUATOR";
  }
  
  written = response.printTo(jsonOut, 255);
  jsonOut[written] = '\n';
  written++;
  jsonOut[written] = '\0';
  reply(jsonOut, written);
}

