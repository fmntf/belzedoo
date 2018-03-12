#include "config.h"
#include <ArduinoJson.h>
#include <StandardCplusplus.h>
#include <Servo.h>
#include <dht.h>
#include <Wire.h>

#include <TSL2561.h>
#include <Adafruit_TCS34725.h>
#include <ChainableLED.h>

#ifdef HAS_BRICK_SUPPORT
#include <SI7021.h> // Humidity Brick
#include <LM75.h> // Temperature Brick
#include <TSL2561.h> // Light Brick
#ifdef ARDUINO_UDOO_NEO
 #include <Adafruit_MPL3115A2_neo_pragma.h> // Barometer Brick
#else
 #include <Adafruit_MPL3115A2_101_pragma.h> // Barometer Brick
#endif
#endif //HAS_BRICK_SUPPORT

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
void comm_reset();

connection_type activeConnection;

#define SERIAL_BUFFER_SIZE 128
#define SERIAL_WATCHDOG_N 256
char serial_buffer[SERIAL_BUFFER_SIZE];
char serial_char;
int  serial_index = 0;
int  serial_available = 0;
bool serial_complete = false;
int  serial_watchdog = SERIAL_WATCHDOG_N;

void setup()
{
  Serial.begin(115200);
#ifdef SERIAL_DEBUG
  SERIAL_DEBUG.begin(115200);
  SERIAL_DEBUG.println("UDOO debug serial started!");
#endif

#ifdef ARDUINO_UDOO_X86
  while (!Serial);
#ifdef SERIAL_DEBUG
  SERIAL_DEBUG.println("Arduino 101 serial is now available");
#endif
#endif
  // empty previous buffer, if any
  while (Serial.available() > 0) Serial.read();

#ifdef HAS_ADK
  activeConnection = CONN_ADK;
  reply("{\"success\":true,\"startup\":true}\n");
#endif
  activeConnection = CONN_SERIAL;
  reply("{\"success\":true,\"startup\":true}\n");
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

  int serial_available = Serial.available();
  if (serial_available > 0) {
#ifdef VERBOSE_DEBUG
      SERIAL_DEBUG.print("Receiving: " );
#endif
    while (serial_available > 0  && !serial_complete) {
      serial_char = Serial.read();
      serial_available--;
#ifdef VERBOSE_DEBUG
      SERIAL_DEBUG.print(serial_char, DEC);
      SERIAL_DEBUG.print(" ");
#endif

      if (serial_char == 255) {
        serial_watchdog--;
        if (serial_watchdog <= 0) {
#ifdef SERIAL_DEBUG
          SERIAL_DEBUG.println("Watchdog reset!");
          serial_watchdog = SERIAL_WATCHDOG_N;
#endif
          return;
        }
      } else {
        if (serial_index > SERIAL_BUFFER_SIZE-1) {
#ifdef SERIAL_DEBUG
          SERIAL_DEBUG.println("Serial buffer overrun!");
#endif
          comm_reset();
          return;
        } // buffer
        
        serial_buffer[serial_index] = serial_char;
        serial_index++;
        serial_buffer[serial_index] = '\0';
        
        if (serial_char == 10 || serial_char == 13) {
          serial_complete = true;
        }
      } // char!=255
    } // while
#ifdef VERBOSE_DEBUG
    SERIAL_DEBUG.println(";");
#endif

    if (serial_complete) {
      activeConnection = CONN_SERIAL;
      processCommand(serial_buffer);
      serial_complete = false;
      serial_index = 0;
    }
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
    else if (root.containsKey("sensor")) {
      handleSensorRequest(root);
    }
    else if (root.containsKey("actuator")) {
      handleActuatorRequest(root);
    }
    else if (root.containsKey("servo")) {
      handleServoRequest(root);
    }
    
  } else {
    int s=0;
    bool found = false;
    while (!found && s<SERIAL_BUFFER_SIZE && readBuffer[s] != '\0') {
#ifdef SERIAL_DEBUG
      SERIAL_DEBUG.println("!");
      SERIAL_DEBUG.println(readBuffer[s]);
#endif
      if (readBuffer[s] == '{') {
        found = true;
        char* substri = readBuffer + s;
        processCommand(substri);
      }
      s++;
    }
    
    if (!found) {
      comm_reset();
      reply("{\"success\":false,\"error\":\"PARSE_FAILED\"}");
      reply(readBuffer);
    }
  }
}

void comm_reset()
{
  serial_index = 0;
  serial_complete = false;
  serial_watchdog = SERIAL_WATCHDOG_N;
  for (int y=0; y<SERIAL_BUFFER_SIZE; y++) serial_buffer[y]=0;
}

