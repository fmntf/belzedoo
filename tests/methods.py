import time
from belzetest import *

test = BelzeTest()
test.openport()


print("Blinking LED 13");
for _ in range(2):
	test.write('{"id": 100, "method":"digitalWrite", "pin":13, "value":1}');
	time.sleep(0.5);
	test.write('{"id": 101, "method":"digitalWrite", "pin":13, "value":0}');
	time.sleep(0.5);

print("Blinking LED 13 (timed)");
for _ in range(5):
	test.write('{"id": 110, "method":"timedDigitalWrite", "pin":13, "value":1, "delay":250}');
	time.sleep(0.25);
	test.write('{"id": 111, "method":"timedDigitalWrite", "pin":13, "value":1, "delay":250}');
	time.sleep(0.25);

print("Digital read");
test.write('{"id": 200, "method":"digitalRead", "pin":10}');

print("Analog read");
test.write('{"id": 201, "method":"analogRead", "pin":0}');

print("Analog read (repeated)");
test.write('{"id": 202, "method":"repeatedAnalogRead", "pin":0, "samples":6, "delay":100}');

print("Analog write");
test.write('{"id": 210, "method":"analogWrite", "pin":9, "value":128}');

print("Delay");
test.write('{"id": 210, "method":"delay", "value":666}');

print("Interrupt (attach)");
test.write('{"id": 220, "method":"attachInterrupt", "pin":3, "mode":2, "interrupt_id":12345}');

print("Interrupt (detach)");
test.write('{"id": 221, "method":"detachInterrupt", "pin":3}');

test.closeport();
