import time
from belzetest import *

test = BelzeTest()
test.openport()

print("I2C Sensors Scanner");
test.write('{"id": 230, "sensor":"scanI2C"}');

for _ in range(0, 3):

    print("Light Brick");
    test.write('{"id": 100, "sensor":"LIGHT_BRICK"}');

    print("Temperature Brick");
    test.write('{"id": 101, "sensor":"TEMPERATURE_BRICK"}');

    print("Humidity Brick");
    test.write('{"id": 102, "sensor":"HUMIDITY_BRICK"}');

    print("Pressure Brick");
    test.write('{"id": 103, "sensor":"PRESSURE_BRICK"}');


test.closeport();
