#include "sensors.h"

extern char jsonOut[256];
extern int written;

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

#ifdef HAS_BRICK_SUPPORT
  else if (strcmp(sensor, "HUMIDITY_BRICK") == 0) {
    
    SI7021 humidityBrick;

    if (humidityBrick.begin()) {
      response["success"]     = (bool)true;
      response["temperature"] = humidityBrick.getCelsiusHundredths()/100;
      response["humidity"]    = humidityBrick.getHumidityPercent();
    } else {
      response["success"] = (bool)false;
      response["errorCode"] = 0;
    }

  }
  
  else if (strcmp(sensor, "PRESSURE_BRICK") == 0) {
    
    Adafruit_MPL3115A2 barometer = Adafruit_MPL3115A2();

    if (barometer.begin()) {
      response["success"]     = (bool)true;
      response["pressure"]    = barometer.getPressure(); // Pascal
      response["altitude"]    = barometer.getAltitude();
      response["temperature"] = barometer.getTemperature();
    } else {
      response["success"] = (bool)false;
      response["errorCode"] = 0;
    }
  }
  
  else if (strcmp(sensor, "TEMPERATURE_BRICK") == 0) {
    
    LM75 tsensor;

    response["success"]     = (bool)true;
    response["temperature"] = tsensor.temp();
  }
  
  else if (strcmp(sensor, "LIGHT_BRICK") == 0) {
     
    TSL2561 tsl(TSL2561_ADDR_LOW); 

    if (tsl.begin()) {
      
      uint32_t lum = tsl.getFullLuminosity();
      uint16_t ir, full;
      ir = lum >> 16;
      full = lum & 0xFFFF;
      
      response["success"] = (bool)true;
      response["visible"] = full - ir;
      response["ir"]      = ir;
      response["full"]    = full;
    } else {
      response["success"] = (bool)false;
      response["errorCode"] = 0;
    }

  }
#endif //HAS_BRICK_SUPPORT
  
  else {
    response["success"] = (bool)false;
    response["error"] = "NO_SENSOR";
  }
  
  written = response.printTo(jsonOut, 255);
  jsonOut[written] = '\n';
  written++;
  jsonOut[written] = '\0';
  reply(jsonOut, written);
}
