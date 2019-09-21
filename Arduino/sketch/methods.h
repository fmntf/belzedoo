#ifndef HEADER_METHODS
#define HEADER_METHODS

#include "config.h"
#include "reply.h"
#include <ArduinoJson.h>
#include "Arduino.h"
#undef min
#undef max
#include <queue>
#include <Wire.h>

namespace std {
  void __throw_bad_alloc()
  {
    Serial.println("Unable to allocate memory");
  }

  void __throw_length_error( char const*e )
  {
    Serial.print("Length Error :");
    Serial.println(e);
  }
}

void handleMethodRequest(JsonObject& root);

void interrupt_handler_pin(int pin);

void interrupt_handler_0();
void interrupt_handler_1();
void interrupt_handler_2();
void interrupt_handler_3();
void interrupt_handler_4();
void interrupt_handler_5();
void interrupt_handler_6();
void interrupt_handler_7();
void interrupt_handler_8();
void interrupt_handler_9();
void interrupt_handler_10();
void interrupt_handler_11();
void interrupt_handler_12();
void interrupt_handler_13();

void flushInterrupts();

#endif
