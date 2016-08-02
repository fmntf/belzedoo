#include "reply.h"

extern char jsonOut[256];
extern int written;
extern connection_type activeConnection;
#ifdef HAS_ADK
extern ADK adk;
#endif

void reply(const char* response, int len)
{
  switch (activeConnection)
  {
#ifdef HAS_ADK
    case CONN_ADK:
      adk.write(len, (uint8_t*)response);
      break;
#endif

    case CONN_SERIAL:
      Serial.println(response);
      break;
  }
  
  #ifdef SERIAL1_DEBUG
        Serial1.print("TX:   ");
        Serial1.println(response);
  #endif
}

void reply(String str)
{
  reply(str.c_str(), str.length());
}
