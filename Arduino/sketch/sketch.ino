#define SERIAL1_DEBUG

#include <adk.h>
#include <ArduinoJson.h>
#include <dht.h>
#include <Wire.h>
#include <Servo.h>
#include "Adafruit_TCS34725.h"

char descriptionName[] = "UDOOAppInventor";
char modelName[] = "AppInventor"; // Need to be the same defined in the Android App
char manufacturerName[] = "UDOO"; // Need to be the same defined in the Android App
char versionNumber[] = "1.0"; // Need to be the same defined in the Android App
char serialNumber[] = "1";
char url[] = "http://appinventor.udoo.org"; // If there isn't any compatible app installed, Android suggest to visit this url

USBHost Usb;
ADK adk(&Usb, manufacturerName, modelName, descriptionName, versionNumber, url, serialNumber);

#define RCVSIZE 128
uint8_t buf[RCVSIZE];
uint32_t bytesRead = 0;
#define SKETCH_VERSION 2

#define CONNECTION_ADK      0
#define CONNECTION_SERIAL   1
int activeConnection;

char jsonOut[256];
int written;
int interrupt_ids[53] = {0};
Servo servos[53];
volatile unsigned long last_interrupt_call[53] = {0};

void setup()
{
  Serial.begin(115200);
#ifdef SERIAL1_DEBUG
  Serial1.begin(115200);
  Serial1.println("UDOO debug serial started!");
#endif
  cpu_irq_enable();
  
  activeConnection = CONNECTION_ADK;
  reply("{\"success\":true,\"startup\":true}");
  activeConnection = CONNECTION_SERIAL;
  reply("{\"success\":true,\"startup\":true}");
}

void loop()
{
  Usb.Task();
   
  if (adk.isReady()) {
    for (int i=0; i<RCVSIZE; i++) {
      buf[i] = '\0';
    }
    
    adk.read(&bytesRead, RCVSIZE, buf);
    if (bytesRead > 0) {
      activeConnection = CONNECTION_ADK;
      processCommand((char*)buf);
    }
  }  

  if (Serial.available() > 0){
    char readFromSerial[100];
    char serialChar;
    int readIndex = 0;
    bool messageComplete = false;
    int watchDog = 512;
    
    while (!messageComplete) {
      serialChar = Serial.read();
      Serial1.print("X+ " );
      Serial1.println(serialChar);
      if (serialChar == 255) {
        watchDog--;
        if (watchDog <= 0) {
          Serial1.println("Watchdog reset!");
          return;
        }
      } else {
        readFromSerial[readIndex] = serialChar;
        readIndex++;
        readFromSerial[readIndex] = '\0';
        
        if (serialChar == 10 || serialChar == 13) {
          messageComplete = true;
        }
      }
    }
    activeConnection = CONNECTION_SERIAL;
    processCommand(readFromSerial);
  }

  delay(10);
}

void processCommand(char* readBuffer)
{
#ifdef SERIAL1_DEBUG
  Serial1.print("CMD: ");
  Serial1.println(readBuffer);
  Serial1.flush();
#endif

  StaticJsonBuffer<2000> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(readBuffer);
  
  if (root.success()) {
    if (root.containsKey("method")) {
      handleMethodRequest(root);
    }
    if (root.containsKey("sensor")) {
      handleSensorRequest(root);
    }
    if (root.containsKey("servo")) {
      handleServoRequest(root);
    }
    
  } else {
    reply("{\"success\":false,\"error\":\"PARSE_FAILED\"}");
    reply(readBuffer);
  }
}

void handleSensorRequest(JsonObject& root)
{
  const char* sensor = root["sensor"];
  StaticJsonBuffer<200> responseJsonBuffer;
  JsonObject& response = responseJsonBuffer.createObject();
  response["id"] = root["id"];

  if (strcmp(sensor, "DHT11") == 0 || strcmp(sensor, "DHT22") == 0) {
    int pin = root["pin"];
    dht DHT;
    int chk;
    bool isDHT11 = false;
    if (strcmp(sensor, "DHT11") == 0) {
      isDHT11 = true;
    }
    
    if (isDHT11) {
      chk = DHT.read11(pin);
    } else {
      chk = DHT.read22(pin);
    }
    if (chk != DHTLIB_OK) {
      // try again, sometimes the sensor returns an error!
      delay(1000);
      if (isDHT11) {
        chk = DHT.read11(pin);
      } else {
        chk = DHT.read22(pin);
      }
    }
        
    if (chk == DHTLIB_OK) {
      response["success"] = (bool)true;
      response["temperature"] = DHT.temperature;
      response["humidity"] = DHT.humidity;
    } else {
      response["success"] = (bool)false;
      response["errorCode"] = chk;
    }
  }
  
  else if (strcmp(sensor, "TCS34725") == 0) {
    
    Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
    if (tcs.begin()) {
      uint16_t clear, red, green, blue;
      tcs.setInterrupt(false);
      delay(60);  // takes 50ms to read 
      tcs.getRawData(&red, &green, &blue, &clear);
      tcs.setInterrupt(true);
      uint32_t sum = clear;
      float r, g, b;
      r = red; r /= sum;
      g = green; g /= sum;
      b = blue; b /= sum;
      r *= 256; g *= 256; b *= 256;
      
      response["success"] = (bool)true;
      response["red"] = (int)r;
      response["green"] = (int)g;
      response["blue"] = (int)b;
    } else {
      response["success"] = (bool)false;
      response["errorCode"] = 0;
    }

  }
  
  else {
    response["success"] = (bool)false;
    response["error"] = "NO_SENSOR";
  }
  
  written = response.printTo(jsonOut, 255);
  jsonOut[written] = '\0';
  reply(jsonOut, written);
}

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
  jsonOut[written] = '\0';
  reply(jsonOut, written);
}

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
    response["disconnected"] = (bool)true;

  } else if (strcmp(method, "digitalWrite") == 0) {
    int pin = root["pin"];
    int value = root["value"];
    digitalWrite(pin, value);

  } else if (strcmp(method, "pinMode") == 0) {
    int pin = root["pin"];
    int value = root["value"];
    pinMode(pin, value);

  } else if (strcmp(method, "digitalRead") == 0) {
    int pin = root["pin"];
    int value = digitalRead(pin);
    response["value"] = value;

  } else if (strcmp(method, "analogRead") == 0) {
    int pin = root["pin"];
    int value = analogRead(pin);
    response["value"] = value;

  } else if (strcmp(method, "map") == 0) {
    int value = root["value"];
    int fromLow = root["fromLow"];
    int fromHigh = root["fromHigh"];
    int toLow = root["toLow"];
    int toHigh = root["toHigh"];
    int mapped = map(value, fromLow, fromHigh, toLow, toHigh);
    response["value"] = mapped;

  } else if (strcmp(method, "analogWrite") == 0) {
    int pin = root["pin"];
    int value = root["value"];
    analogWrite(pin, value);

  } else if (strcmp(method, "delay") == 0) {
    int value = root["value"];
    delay(value);
    
  } else if (strcmp(method, "attachInterrupt") == 0) {
    int pin = root["pin"];
    int mode = root["mode"]; //change=2, falling=3, rising=4
    interrupt_ids[pin] = root["interrupt_id"];

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

#ifdef SERIAL1_DEBUG    
    Serial1.print("Registered interrupt handler on pin ");
    Serial1.print(pin);
    Serial1.print(" and mode ");
    Serial1.print(mode);
    Serial1.print(" widh id ");
    Serial1.println(interrupt_ids[pin]);
#endif
  }
 
  else {
    response["success"] = (bool)false;
    response["error"] = "NO_METHOD";
  }
  
  written = response.printTo(jsonOut, 255);
  jsonOut[written] = '\0';
  reply(jsonOut, written);
}

void interrupt_handler_pin(int pin)
{
  if (millis() - last_interrupt_call[pin] > 100) {
    last_interrupt_call[pin] = millis();
#ifdef SERIAL1_DEBUG
    Serial1.println("Called INT HANDLER!");
#endif  
    StaticJsonBuffer<200> responseJsonBuffer;
    JsonObject& response = responseJsonBuffer.createObject();
    response["success"] = (bool)true;
    response["pin"] = pin;
    response["id"] = interrupt_ids[pin];
    
    written = response.printTo(jsonOut, 255);
    jsonOut[written] = '\0';
    
    reply(jsonOut, written);
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

void reply(const char* response, int len)
{
  switch (activeConnection)
  {
    case CONNECTION_ADK:
      adk.write(len, (uint8_t*)response);
      break;

    case CONNECTION_SERIAL:
      Serial.println(response);
#ifdef SERIAL1_DEBUG
      Serial1.println(response);
#endif
      break;
  }
}

void reply(String str)
{
  reply(str.c_str(), str.length());
}

