#include "config.h"
#include <ArduinoJson.h>
#include <StandardCplusplus.h>
#include <Servo.h>
#include <dht.h>
#include <Wire.h>

#include <TSL2561.h>
#include <Adafruit_TCS34725.h>
#include <ChainableLED.h>

#include <SI7021.h> // Humidity Brick
#include <LM75.h> // Temperature Brick
#include <TSL2561.h> // Light Brick

#ifdef ARDUINO_UDOO_NEO
#include <Adafruit_MPL3115A2_neo_pragma.h> // Barometer Brick
#else
#include <Adafruit_MPL3115A2.h> // Barometer Brick
#endif

#ifdef HAS_ADK
#include <adk.h>
char descriptionName[]  = "UDOOAppInventor";
char modelName[]        = "AppInventor"; // Need to be the same defined in the Android App
char manufacturerName[] = "UDOO"; // Need to be the same defined in the Android App
char versionNumber[   ] = "1.0"; // Need to be the same defined in the Android App
char serialNumber[]     = "1";
char url[]              = "http://appinventor.udoo.org"; // If there isn't any compatible app installed, Android suggest to visit this url

USBHost usb;
ADK adk(&usb, manufacturerName, modelName, descriptionName, versionNumber, url, serialNumber);
#endif //HAS_ADK

#define RCVSIZE 128
uint8_t buf[RCVSIZE];
uint32_t bytesRead = 0;
char jsonOut[256];
int written;

#include "reply.h"
#include "sensors.h"
#include "actuators.h"
#include "methods.h"
#include "servo.h"

void processCommand(char* readBuffer);

connection_type activeConnection;


void setup()
{
  Serial.begin(115200);
#ifdef SERIAL_DEBUG
  SERIAL_DEBUG.begin(115200);
  SERIAL_DEBUG.println("UDOO debug serial started!");
#endif

#ifdef HAS_ADK
  activeConnection = CONN_ADK;
  reply("{\"success\":true,\"startup\":true}");
#endif
  activeConnection = CONN_SERIAL;
  reply("{\"success\":true,\"startup\":true}");
}

void loop()
{
#ifdef HAS_ADK
  usb.Task();
   
  if (adk.isReady()) {
    for (int i=0; i<RCVSIZE; i++) {
      buf[i] = '\0';
    }
    
    adk.read(&bytesRead, RCVSIZE, buf);
    if (bytesRead > 0) {
      activeConnection = CONN_ADK;
      processCommand((char*)buf);
    }
  }
#endif //HAS_ADK

  if (Serial.available() > 0){
    char readFromSerial[100];
    char serialChar;
    int readIndex = 0;
    bool messageComplete = false;
    int watchDog = 512;
    
    while (!messageComplete) {
      serialChar = Serial.read();
#ifdef SERIAL_DEBUG
      SERIAL_DEBUG.print("X+ " );
      SERIAL_DEBUG.println(serialChar);
#endif
      if (serialChar == 255) {
        watchDog--;
        if (watchDog <= 0) {
#ifdef SERIAL_DEBUG
          SERIAL_DEBUG.println("Watchdog reset!");
#endif
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
    activeConnection = CONN_SERIAL;
    processCommand(readFromSerial);
  }

  flushInterrupts();

  delay(10);
}

void processCommand(char* readBuffer)
{
#ifdef SERIAL_DEBUG
  SERIAL_DEBUG.print("RX:   ");
  SERIAL_DEBUG.println(readBuffer);
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
    if (root.containsKey("actuator")) {
      handleActuatorRequest(root);
    }
    if (root.containsKey("servo")) {
      handleServoRequest(root);
    }
    
  } else {
    reply("{\"success\":false,\"error\":\"PARSE_FAILED\"}");
    reply(readBuffer);
  }
}


