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

#define CONNECTION_ADK      0
#define CONNECTION_SERIAL   1
int activeConnection;

char successResponse[] = "{\"success\":true}";
String successReply = "{\"success\":true}";

void setup()
{
    Serial.begin(115200);   
    Serial.println("UDOO Serial started!");
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
    
    if (Serial.available()){
      char readFromSerial[100];
      char serialChar;
      int readIndex = 0;
      bool messageComplete = false;
      
      while (!messageComplete) {
        serialChar = Serial.read();
        if (serialChar != 255) {
          readFromSerial[readIndex] = serialChar;
          readIndex++;
          readFromSerial[readIndex] = '\0';
        }
        
        if (serialChar == 10 || serialChar == 13) {
          messageComplete = true;
        }
      }
      activeConnection = CONNECTION_SERIAL;
      processCommand(readFromSerial);
    }
    
    delay(10);
}

void processCommand(char* readBuffer)
{
//  Serial.print("CMD: ");
//  Serial.println(readBuffer);
//  Serial.flush();
  
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
  }
}

void callSensor(JsonObject& root)
{
  const char* sensor = root["sensor"];

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
    
    StaticJsonBuffer<200> responseJsonBuffer;
    JsonObject& response = responseJsonBuffer.createObject();
    
    if (chk == DHTLIB_OK) {
      response["success"] = (bool)true;
      response["temperature"] = DHT.temperature;
      response["humidity"] = DHT.humidity;
    } else {
      response["success"] = (bool)false;
      response["errorCode"] = chk;
    }
    
    char jsonOut[128];
    int written = response.printTo(jsonOut, 128);
    reply(jsonOut, written);
  }
  
  else {
    reply("{\"success\":false,\"error\":\"NO_SENSOR\"}");
  }
}

void callMethod(JsonObject& root)
{
  const char* method = root["method"];

  if (strcmp(method, "hi") == 0) {
    reply(successReply);
    
  } else if (strcmp(method, "digitalWrite") == 0) {
    int pin = root["pin"];
    int value = root["value"];
    digitalWrite(pin, value);
    reply(successReply);

  } else if (strcmp(method, "pinMode") == 0) {
    int pin = root["pin"];
    int value = root["value"];
    pinMode(pin, value);
    reply(successReply);

  } else if (strcmp(method, "digitalRead") == 0) {
    int pin = root["pin"];
    int value = digitalRead(pin);     
    StaticJsonBuffer<200> responseJsonBuffer;
    JsonObject& response = responseJsonBuffer.createObject();
    response["success"] = (bool)true;
    response["value"] = value;
    char jsonOut[128];
    int written = response.printTo(jsonOut, 128);
    reply(jsonOut, written);

  } else if (strcmp(method, "analogRead") == 0) {
    int pin = root["pin"];
    int value = analogRead(pin);
    StaticJsonBuffer<200> responseJsonBuffer;
    JsonObject& response = responseJsonBuffer.createObject();
    response["success"] = (bool)true;
    response["value"] = value;
    char jsonOut[128];
    int written = response.printTo(jsonOut, 128);
    reply(jsonOut, written);

  } else if (strcmp(method, "map") == 0) {
    int value = root["value"];
    int fromLow = root["fromLow"];
    int fromHigh = root["fromHigh"];
    int toLow = root["toLow"];
    int toHigh = root["toHigh"];
    int mapped = map(value, fromLow, fromHigh, toLow, toHigh);
    StaticJsonBuffer<200> responseJsonBuffer;
    JsonObject& response = responseJsonBuffer.createObject();
    response["success"] = (bool)true;
    response["value"] = mapped;
    char jsonOut[128];
    int written = response.printTo(jsonOut, 128);
    reply(jsonOut, written);

  } else if (strcmp(method, "analogWrite") == 0) {
    int pin = root["pin"];
    int value = root["value"];
    analogWrite(pin, value);
    reply(successReply);

  } else if (strcmp(method, "delay") == 0) {
    int value = root["value"];
    delay(value);
    reply(successReply);
  }
 
  else {
    reply("{\"success\":false,\"error\":\"NO_METHOD\"}");
  }
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
      break;
  }
}

void reply(String str)
{
  reply(str.c_str(), str.length());
}

