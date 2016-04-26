#include <Wire.h>
#include <SI7021.h>

SI7021 sensor;

void setup() {
	Serial.begin(9600);
    sensor.begin();
}


void loop() {

    // temperature is an integer in hundredths
    int temperature = sensor.getCelsiusHundredths();
    temperature = temperature / 100;
    
    // humidity is an integer representing percent
    int humidity = sensor.getHumidityPercent();

    // this driver should work for SI7020 and SI7001
    int deviceid = sensor.getDeviceId();

    // enable internal heater for testing
    //sensor.setHeater(true);
    //delay(20000);
    //sensor.setHeater(false);

    // get humidity and temperature in one shot, saves power because sensor takes temperature when doing humidity anyway
    si7021_env data = sensor.getHumidityAndTemperature();

    Serial.print("Temp: ");
    Serial.print(data.celsiusHundredths/100);
    Serial.print("  /  hum: "));
    Serial.println(data.humidityPercent);
    delay(1000);
}




