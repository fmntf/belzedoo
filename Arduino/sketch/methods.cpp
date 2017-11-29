#include "methods.h"

extern char jsonOut[256];
extern int written;

int interrupt_ids[53] = {0};
volatile unsigned long last_interrupt_call[53] = {0};

std::queue <response_t> queuedInterrupts;

void handleMethodRequest(JsonObject& root)
{
  const char* method = root["method"];
  StaticJsonBuffer<200> responseJsonBuffer;
  JsonObject& response = responseJsonBuffer.createObject();
  response["success"] = (bool)true;
  response["id"] = root["id"];

  if (strcmp(method, "hi") == 0) {
    response["version"] = SKETCH_VERSION;
    
  } else if (strcmp(method, "disconnect") == 0) {
    for (int i=0; i<=13; i++) {
      detachInterrupt(i);
    }
    response["disconnected"] = (bool)true;

  } else if (strcmp(method, "digitalWrite") == 0) {
    int pin = root["pin"];
    int value = root["value"];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, value);

  } else if (strcmp(method, "timedDigitalWrite") == 0) {
    int pin = root["pin"];
    int value = root["value"];
    int delayTime = root["delay"];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, value);
    delay(delayTime);
    digitalWrite(pin, !value);

  } else if (strcmp(method, "pinMode") == 0) {
    int pin = root["pin"];
    int value = root["value"];
    pinMode(pin, value);
    response["deprecated"] = (bool)true;

  } else if (strcmp(method, "digitalRead") == 0) {
    int pin = root["pin"];
    pinMode(pin, INPUT);
    int value = digitalRead(pin);
    response["value"] = value;

  } else if (strcmp(method, "analogRead") == 0) {
    int pin = root["pin"];
    int value = analogRead(pin);
    response["value"] = value;

  } else if (strcmp(method, "repeatedAnalogRead") == 0) {
    int pin = root["pin"];
    int n = root["samples"];
    int d = root["delay"];
    JsonArray& samples = response.createNestedArray("samples");
    for (int i=0; i<n; i++) {
      samples.add(analogRead(pin));
      delay(d);
    }

  } else if (strcmp(method, "map") == 0) {
    int value = root["value"];
    int fromLow = root["fromLow"];
    int fromHigh = root["fromHigh"];
    int toLow = root["toLow"];
    int toHigh = root["toHigh"];
    int mapped = map(value, fromLow, fromHigh, toLow, toHigh);
    response["value"] = mapped;
    response["deprecated"] = (bool)true;

  } else if (strcmp(method, "analogWrite") == 0) {
    int pin = root["pin"];
    int value = root["value"];
    pinMode(pin, OUTPUT);
    analogWrite(pin, value);

  } else if (strcmp(method, "delay") == 0) {
    int value = root["value"];
    delay(value);
    
  } else if (strcmp(method, "attachInterrupt") == 0) {
    int pin = root["pin"];
    int mode = root["mode"]; //change=2, falling=3, rising=4
    interrupt_ids[pin] = root["interrupt_id"];

   #ifdef ARDUINO_UDOO_X86
   if (mode == 2 && (
     pin == 0 || pin == 1 || pin == 3 || pin == 4 || pin == 6 || pin == 9
   )) {
     response["success"] = (bool)false;
     response["error"] = "CHANGE unavailable on 0/1/3/4/6/9";
   }
   #endif

    if (response["success"]) {
      pinMode(pin, INPUT);
      last_interrupt_call[pin] = millis();
      switch (pin) {
        case 0: attachInterrupt(pin, interrupt_handler_0, mode); break;
        case 1: attachInterrupt(pin, interrupt_handler_1, mode); break;
        case 2: attachInterrupt(pin, interrupt_handler_2, mode); break;
        case 3: attachInterrupt(pin, interrupt_handler_3, mode); break;
        case 4: attachInterrupt(pin, interrupt_handler_4, mode); break;
        case 5: attachInterrupt(pin, interrupt_handler_5, mode); break;
        case 6: attachInterrupt(pin, interrupt_handler_6, mode); break;
        case 7: attachInterrupt(pin, interrupt_handler_7, mode); break;
        case 8: attachInterrupt(pin, interrupt_handler_8, mode); break;
        case 9: attachInterrupt(pin, interrupt_handler_9, mode); break;
        case 10: attachInterrupt(pin, interrupt_handler_10, mode); break;
        case 11: attachInterrupt(pin, interrupt_handler_11, mode); break;
        case 12: attachInterrupt(pin, interrupt_handler_12, mode); break;
        case 13: attachInterrupt(pin, interrupt_handler_13, mode); break;
      }
    }

#ifdef SERIAL_DEBUG
    char buff[100];
    snprintf(buff, sizeof(buff), "Registered interrupt handler on pin %d and mode %d with ID=%d", 
      pin, mode, interrupt_ids[pin]);
    SERIAL_DEBUG.println(buff);
#endif

  } else if (strcmp(method, "detachInterrupt") == 0) {
    int pin = root["pin"];
    interrupt_ids[pin] = 0;
    detachInterrupt(pin);
    
  } else if (strcmp(method, "scanSensors") == 0) {

    Wire.begin();
    JsonArray& sensors = response.createNestedArray("sensors");

    if (check_i2c_device(0x29, 0x0A) == 0) {
      sensors.add("LIGHT_BRICK");
    }
    if (check_i2c_device(0x40, 0xE5) == 0) {
      sensors.add("HUMIDITY_BRICK");
    }
    if (check_i2c_device(0x48, 0x00) == 0) {
      sensors.add("TEMPERATURE_BRICK");
    }
    if (check_i2c_device(0x60, 0x00) == 0) {
      sensors.add("PRESSURE_BRICK");
    }
  }
 
  else {
    response["success"] = (bool)false;
    response["error"] = "NO_METHOD";
  }
  
  written = response.printTo(jsonOut, 255);
  jsonOut[written] = '\n';
  written++;
  jsonOut[written] = '\0';
  reply(jsonOut, written);
}

int check_i2c_device(int dev_address, int reg) {
  Wire.beginTransmission(dev_address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(dev_address, 1);
  int v = Wire.read();
  if (v != 0xFFFFFFFF) {
    return 0;
  }
  return -1;
}

void interrupt_handler_pin(int pin)
{
  if (millis() - last_interrupt_call[pin] > 100) {
    last_interrupt_call[pin] = millis();
#ifdef SERIAL_DEBUG
    SERIAL_DEBUG.println("Called interrupt handler!");
#endif  
    StaticJsonBuffer<200> responseJsonBuffer;
    JsonObject& response = responseJsonBuffer.createObject();
    response["success"] = (bool)true;
    response["pin"] = pin;
    response["id"] = interrupt_ids[pin];
    response["timestamp"] = last_interrupt_call[pin];

    written = response.printTo(jsonOut, 255);
    jsonOut[written] = '\n';
    written++;
    jsonOut[written] = '\0';

    response_t res = {written, jsonOut};
    queuedInterrupts.push(res);
  }
}

void flushInterrupts()
{
  while(!queuedInterrupts.empty()) {
    response_t res = queuedInterrupts.front();
    reply(res.json, res.length);
    queuedInterrupts.pop();
  }
}

void interrupt_handler_0() {interrupt_handler_pin(0);}
void interrupt_handler_1() {interrupt_handler_pin(1);}
void interrupt_handler_2() {interrupt_handler_pin(2);}
void interrupt_handler_3() {interrupt_handler_pin(3);}
void interrupt_handler_4() {interrupt_handler_pin(4);}
void interrupt_handler_5() {interrupt_handler_pin(5);}
void interrupt_handler_6() {interrupt_handler_pin(6);}
void interrupt_handler_7() {interrupt_handler_pin(7);}
void interrupt_handler_8() {interrupt_handler_pin(8);}
void interrupt_handler_9() {interrupt_handler_pin(9);}
void interrupt_handler_10() {interrupt_handler_pin(10);}
void interrupt_handler_11() {interrupt_handler_pin(11);}
void interrupt_handler_12() {interrupt_handler_pin(12);}
void interrupt_handler_13() {interrupt_handler_pin(13);}

