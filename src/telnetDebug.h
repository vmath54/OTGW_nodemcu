#ifndef _TELNETDEBUG_H_
#define _TELNETDEBUG_H_

#include <ESP8266WiFiMulti.h>
#include "config.h"

#define DEBUG_USAGE                    "start stop dbg on|off  msgs on|off  errors"

#ifdef TELNET_DEBUG_PRINT
  #define d_print(x)            if (telnet_debug_available()) {telnet_debug_client.print(x);}
  #define d_println(x)          if (telnet_debug_available()) {telnet_debug_client.println(x);}
  #define d_printf(format,...)  if (telnet_debug_available()) {telnet_debug_client.printf(format, __VA_ARGS__);}
     // ex : d_printf("example %d, %s%s", 5, "toto", EOL)
#else    // TELNET_DEBUG_PRINT
  #define d_print(x)
  #define d_println(x)
  #define d_printf(...)
#endif   // TELNET_DEBUG_PRINT


extern bool telnet_debug_available(void);
extern void printDebugHex(const char *, const char *, int, bool stopOnEOF = true);
extern String decodeHex(const char *, int, bool stopOnEOF = true);

#ifdef USE_TELNET_DEBUG

extern WiFiClient telnet_debug_client;

extern void telnet_debug_server_setup(void);
extern void telnet_debug_server_loop(void);
extern void telnet_debug_print_frames (void);

#endif  // USE_TELNET_DEBUG


#endif  // _TELNETDEBUG_H_