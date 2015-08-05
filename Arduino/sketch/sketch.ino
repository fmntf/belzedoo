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

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200);
    Serial1.println("UDOO debug serial started!");
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
  Serial1.print("CMD: ");
  Serial1.println(readBuffer);
  Serial1.flush();
  
  if (strcmp(readBuffer, "H") == 0) {
    reply("I");
    return;
  }
  
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
  
  char jsonOut[128];
  int written = response.printTo(jsonOut, 128);
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
  }
 
  else {
    response["success"] = (bool)false;
    response["error"] = "NO_METHOD";
  }
  
  char jsonOut[128];
  int written = response.printTo(jsonOut, 128);
  jsonOut[written] = '\0';
  reply(jsonOut, written);
}

void reply(const char* response, int len)
{
  switch (activeConnection)
  {
    case CONNECTION_ADK:
      adk.write(len, (uint8_t*)response);
      break;

    case CONNECTION_SERIAL:
      Serial.println(response);
      Serial1.println(response);
      break;
  }
}

void reply(String str)
{
  reply(str.c_str(), str.length());
}

