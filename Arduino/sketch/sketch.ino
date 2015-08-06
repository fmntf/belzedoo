#define SERIAL1_DEBUG

#include <adk.h>
#include <ArduinoJson.h>
#include <dht11.h>

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
volatile unsigned long last_interrupt_call[53] = {0};

void setup()
{
  Serial.begin(115200);
#ifdef SERIAL1_DEBUG
  Serial1.begin(115200);
  Serial1.println("UDOO debug serial started!");
#endif
  cpu_irq_enable();
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
      callMethod(root);
    }
    if (root.containsKey("sensor")) {
      callSensor(root);
    }
    
  } else {
    reply("{\"success\":false,\"error\":\"PARSE_FAILED\"}");
    reply(readBuffer);
  }
}

void callSensor(JsonObject& root)
{
  const char* sensor = root["sensor"];
  StaticJsonBuffer<200> responseJsonBuffer;
  JsonObject& response = responseJsonBuffer.createObject();
  response["id"] = root["id"];

  if (strcmp(sensor, "dht11") == 0) {
    int pin = root["pin"];
    dht11 DHT;
    int chk;
    chk = DHT.read(pin);
    
    if (chk != DHTLIB_OK) {
      // try again, sometimes the sensor returns an error!
      delay(1000);
      chk = DHT.read(pin);
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
  
  else {
    response["success"] = (bool)false;
    response["error"] = "NO_SENSOR";
  }
  
  written = response.printTo(jsonOut, 255);
  jsonOut[written] = '\0';
  reply(jsonOut, written);
}

void callMethod(JsonObject& root)
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

