import time
from belzetest import *

test = BelzeTest()
test.openport()


print("Interrupt 5 CHANGE");
test.write('{"id": 100, "method":"attachInterrupt", "mode":2, "pin":5}');
time.sleep(1);

print("Interrupt 6 CHANGE (expected failure on X86)");
test.write('{"id": 101, "method":"attachInterrupt", "mode":2, "pin":6}');
time.sleep(1);

print("Interrupt 7 RISING");
test.write('{"id": 102, "method":"attachInterrupt", "mode":4, "pin":7}');


test.closeport();
