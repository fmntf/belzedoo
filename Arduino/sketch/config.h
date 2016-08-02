#ifndef HEADER_CONFIG
#define HEADER_CONFIG

#define UDOOQuad
//#define UDOONeo

#define SKETCH_VERSION 3


#ifdef UDOOQuad
#define SERIAL_DEBUG Serial1
#define HAS_ADK
#endif

#ifdef UDOONeo
#define SERIAL_DEBUG Serial0
#endif

#endif
