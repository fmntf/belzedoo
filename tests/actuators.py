import time
from belzetest import *

test = BelzeTest()
test.openport()


print("RGB rainbow (fast)");
test.write('{"id": 100, "actuator":"P9813", "pin":8, "mode":"rainbow", "n":3, "delay":5}', False);
time.sleep(6);
test.read();

print("RGB rainbow (slow)");
test.write('{"id": 100, "actuator":"P9813", "pin":8, "mode":"rainbow", "n":2, "delay":50}', False);
time.sleep(12);
test.read();

print("RGB rainbow (red)");
test.write('{"id": 100, "actuator":"P9813", "pin":8, "mode":"rgb", "r":255, "g":0, "b":0}', False);
time.sleep(1);
test.read();

print("RGB rainbow (off)");
test.write('{"id": 100, "actuator":"P9813", "pin":8, "mode":"rgb", "r":0, "g":0, "b":0}');


test.closeport();
