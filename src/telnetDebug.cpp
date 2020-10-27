#include <Arduino.h>
#include <ESP8266WiFiMulti.h>

#include "telnetDebug.h"
#include "OTGWprocessData.h"

#if defined(USE_TELNET_DEBUG)

// ################### internal variables ############################

WiFiServer telnet_debug_server(telnet_debug_port);
WiFiClient telnet_debug_client;

int dbgLevel = 0;
int printOTGWmsgs = 0;
unsigned int lastOTGWmsgNumber = 0;  // dernier message ecrit.

// ################### internal functions ############################

void printErrors(WiFiClient client) {
  lastErrors * lastErrors1 = getErrors();

  if (lastErrors1->nb == 0) client.println("no error");
  else {
    client.printf("%sTotal erreurs : %d%s", EOL, lastErrors1->nb, EOL);
    for (int i = 0; i < MAX_TYPE_ERRORS; i++) {
      if (lastErrors1->nbErrorsByType[i] != 0)
      {
        client.printf("    %d : %s%s", lastErrors1->nbErrorsByType[i], lastErrors1->msg[i] ,EOL);
      }
    }
    client.println("");
  }
}

void parse_debug_cmd(WiFiClient client) {
  String cmd;
  cmd.reserve(16);

  client.setTimeout(1); // Max waiting time for readStringUntil()
  cmd = client.readStringUntil('\r');
  client.readStringUntil('\n'); // Discard LF

  if (cmd.length() < 4) {
    client.println(DEBUG_USAGE);
    return;
  }

  if (cmd.equals("help")) {
    client.println(DEBUG_USAGE);
    return;
  } else if (cmd.equals("start")) {
    client.println("Start process data ...");
    cleanErrors(); d_runProcessData(true); loop_restart();  } 
    else if (cmd.equals("stop")) {
    client.println("Stop process data ...");
    d_runProcessData(false);  } 
    else if (cmd.equals("dbg on")) { 
    client.println("Start debug ...");
    //loop_restart();
    dbgLevel = 1;
  } else if (cmd.equals("dbg off")) {
    client.println("Stop debug ...");
    dbgLevel = 0;
  } else if (cmd.equals("msgs on")) { 
    client.println("Start print OTGW messages ...");
    printOTGWmsgs = 1;
  } else if (cmd.equals("msgs off")) {
    client.println("Stop print OTGW messages ...");
    printOTGWmsgs = 0;  
  } else if (cmd.equals("errors")) { 
    printErrors(client);
/*
  } else if (cmd.equals("essai")) { 
    pushAction(ESSAI); pushAction(SUMMARY_REQUEST);
*/
  } else {
    client.println(DEBUG_USAGE);
  }
}

// ################### exported functions ############################

bool telnet_debug_available(void)
{
  return ((dbgLevel) && (telnet_debug_client));
}

void telnet_debug_server_setup(void) {
  telnet_debug_server.begin();
}

void telnet_debug_server_loop(void) {
  // Get new debug client
  if (telnet_debug_server.hasClient()) {
    if (!telnet_debug_client) {
      telnet_debug_client = telnet_debug_server.available();
      s_println("SRV > New OTGW_stat client");
    }
    else { 
      WiFiClient a_client_stat = telnet_debug_server.available();
      a_client_stat.printf_P(ERR_SERVER_BUSY, 1, EOL);
      s_println("SRV > Max clients debug exceeded. Rejecting!");
    }
  }

  // STAT commands
  if (telnet_debug_client.available()) {
    parse_debug_cmd(telnet_debug_client);
  }

  // print OTGW messages
#ifdef OTGW_PROCESS_DATA
  lastOTGWmsg * OTGWmsg = getOTGWmsg();
  if ((printOTGWmsgs) && (OTGWmsg->nb != lastOTGWmsgNumber)) {
    lastOTGWmsgNumber = OTGWmsg->nb;
    telnet_debug_client.println(OTGWmsg->msg); 
  }
#endif // OTGW_PROCESS_DATA
}

void printDebugHex(const char *comment, const char *buf, int sizeMax, bool stopOnEOF) {
    String res = "";
    uint8_t currentByte;
    int rc = 0;

  if (telnet_debug_available()) {

    if (comment[0] != '\0') telnet_debug_client.printf("---- %s ----%s", comment, EOL);

    for (int i = 0; i < sizeMax; i++) {
      rc++;
      currentByte = buf[i];
      if (currentByte == 0 && stopOnEOF) {   // End Of String
        telnet_debug_client.println(res);
        return;
      }

      if (currentByte < 16 ) res += "0";
      res += String(currentByte, HEX);
      res += " ";

      if (((rc % 20) == 0) || (currentByte == 10)) {
        telnet_debug_client.println(res);
        rc = 0;
        res = "";
      }
    }
    telnet_debug_client.println(res);
  } 
}


String decodeHex(const char *msg, int sizeMax, bool stopOnEOF)
{
  String res = "";
  int rc = 0;

  for (int i = 0; i < sizeMax; i++) {
    rc++;
    if (msg[i] == '\0' && stopOnEOF) {
      return res;
    }
    int currentChar = (int) msg[i];
    if (currentChar < 40) res += "0";
    res += String(currentChar, HEX);
    res += ' ';
    if (((rc % 20) == 0) || (currentChar == 10)) {
      res += "\r\n";
      rc = 0;
    }
  }

  return res;
}

/*
void telnet_debug_printf(const char *format, ...) {
  if ((dbgLevel) && telnet_debug_client) {
    char buffer[256];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer,256,format, args);
    va_end (args);
    telnet_debug_client.print(buffer);
  }
}
*/

#else  // USE_TELNET_DEBUG

String decodeHex(char *msg, int sizeMax, bool stopOnEOF)
{
  return "";
}

#endif  // USE_TELNET_DEBUG