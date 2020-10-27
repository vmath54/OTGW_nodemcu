#include <Arduino.h>
#include "config.h"
#include "httpServ.h"
#include "telnetDebug.h"
#include "OTGWprocessData.h"

#if defined(USE_HTTP_SERVER)

// ################### internal variables ############################

ESP8266WebServer http_server ( http_server_port );

// ################### internal functions ############################

void handleHttpNotFound() {
  String msg = "File Not Found\n\n";
  msg += "URI: ";
  msg += http_server.uri();
  msg += "\nMethod: ";
  msg += (http_server.method() == HTTP_GET) ? "GET" : "POST";
  msg += "\nArguments: ";
  msg += http_server.args();
  msg += "\n";
  for (uint8_t i = 0; i < http_server.args(); i++) {
    msg += " " + http_server.argName(i) + ": " + http_server.arg(i) + "\n";
  }
  http_server.send(404, "text/plain", msg);
}

void handleHttpErrors(){
  lastErrors * lastErrors1 = getErrors();

  String msg = "";
  if (lastErrors1->nb == 0) msg += ("no error");
  else {
    msg += "total errors : ";  msg += lastErrors1->nb; msg += EOL;
    for (int i = 0; i < MAX_TYPE_ERRORS; i++) {
      if (lastErrors1->nbErrorsByType[i] != 0)
      {
        msg += lastErrors1->nbErrorsByType[i]; msg += " -> ";  msg += lastErrors1->msg[i]; msg += EOL;
      }
    }
  }
  http_server.send(200, "text/plain", msg);
}

void handleHttpSummary() {
  String msg = "";

#ifdef OTGW_PROCESS_SUMMARY
  lastOTGWsummary * summary = getSummary();
  msg += summary->msg; msg += EOL; msg += summary->nb; msg += EOL;
#endif // OTGW_PROCESS_SUMMARY 
  http_server.send(200, "text/plain", msg);
}

void handleHttpRoot() { 
  http_server.send(200, "text/html", "<html>\n<head><title>OpenTerm GateWay</title></head>\n<body>\n<ul><li><a href=\"/errors\">errors</a></li><li><a href=\"/summary\">summary</a></li></ul>\n</body>\n</html>");
}


// ################### exported functions ############################

void http_server_setup(void) {
  http_server.on("/", handleHttpRoot);
  http_server.on("/errors", handleHttpErrors);
  http_server.onNotFound(handleHttpNotFound);
#ifdef OTGW_PROCESS_SUMMARY
  http_server.on("/summary", handleHttpSummary);
#endif

  http_server.begin();
  s_println(F("HTTP server started"));
}

void http_server_loop(void) {
    http_server.handleClient();
}

#endif  // USE_HTTP_SERVER