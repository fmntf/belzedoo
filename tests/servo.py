import time
from belzetest import *

test = BelzeTest()
test.openport()


print("Servo 0 degrees");
test.write('{"id": 100, "servo":"write", "degrees":0, "pin":6}');
time.sleep(2);

print("Servo 10 degrees");
test.write('{"id": 101, "servo":"write", "degrees":10, "pin":6}');
time.sleep(2);

print("Servo 90 degrees");
test.write('{"id": 102, "servo":"write", "degrees":90, "pin":6}');
time.sleep(2);

print("Servo 180 degrees");
test.write('{"id": 103, "servo":"write", "degrees":180, "pin":6}');
time.sleep(2);

print("Servo 60 degrees");
test.write('{"id": 104, "servo":"write", "degrees":60, "pin":6}');
time.sleep(0.5);

print("Servo 60 degrees");
test.write('{"id": 104, "servo":"write", "degrees":60, "pin":6}');
time.sleep(0.5);

print("Servo 60 degrees");
test.write('{"id": 104, "servo":"write", "degrees":60, "pin":6}');


test.closeport();
