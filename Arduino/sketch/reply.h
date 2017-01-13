#ifndef HEADER_REPLY
#define HEADER_REPLY

#include "config.h"
#include "Arduino.h"
#ifdef HAS_ADK
#include <adk.h>
#endif

typedef struct {
  int length;
  char* json;
} response_t;

typedef enum {
#ifdef HAS_ADK
  CONN_ADK,
#endif
  CONN_SERIAL
} connection_type;

void reply(const char* response, int len);

void reply(String str);

#endif
