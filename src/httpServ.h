#ifndef _HTTPSERV_H_
#define _HTTPSERV_H_

#include "config.h"

#ifdef USE_HTTP_SERVER
#include <ESP8266WebServer.h>
extern void http_server_setup(void);
extern void http_server_loop(void);

#endif  // USE_HTTP_SERVER


#endif  // _OTGWHTTP_H_