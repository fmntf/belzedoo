import time
from belzetest import *

test = BelzeTest()
test.openport()


print("Hi");
test.write('{"id": 200, "method":"hi"}');

print("Hi");
test.write('{"id": 201, "method":"digitalWrite", "pin":13, "value":0}'+chr(10)+'{"id": 202, "method":"hi"}');
test.read();

print("Hi");
test.write('}{"id": 203, "method":"hi"}');

print("Hi");
test.write('{"id": 204, "method":"hi"}{');

print("Blinking LED 13");
test.write('{"id": 100, "method":"digitalWrite", "pin":13, "value":1}');
time.sleep(0.2);
test.write('{"id": 101, "method":"digitalWrite", "pin":13, "value":0}');

test.closeport();
