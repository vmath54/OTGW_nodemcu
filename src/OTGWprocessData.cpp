#include <Arduino.h>

//#include "config.h"
#include "OTGWprocessData.h"
#include "telnetDebug.h"

lastErrors lastErrors1;

lastErrors * getErrors(void) {
  return &lastErrors1;
}

lastOTGWmsg lastOTGWmsg1;
bool processData = false;

lastOTGWmsg * getOTGWmsg(void) {
  return &lastOTGWmsg1;
}

lastOTGWsummary lastOTGWsummary1;

lastOTGWsummary * getSummary(void) {
  return &lastOTGWsummary1;
}

void recordErrors(const char * msg) {
  lastErrors1.nb++;

  for (int i = 0; i < MAX_TYPE_ERRORS; i++) {
    if (lastErrors1.nbErrorsByType[i] == 0) {
      lastErrors1.msg[i] = msg;
      lastErrors1.nbErrorsByType[i] = 1;
      return;
    }

    if (strncmp(lastErrors1.msg[i], msg, 40) == 0)
    {
      lastErrors1.nbErrorsByType[i]++;
      return;
    }

    return;
  }
}

void cleanErrors() {
  lastErrors1.nb = 0;
  for (int i = 0; i < MAX_TYPE_ERRORS; i++)
  {
    lastErrors1.msg[i] = NULL;
    lastErrors1.nbErrorsByType[i] = 0;
  }
}

void OTGWprocessData_setup(void) {
#ifdef OTGW_PROCESS_DATA_ON_BOOT
  processData = true;
#endif
  cleanErrors();
}


#if defined (OTGW_PROCESS_DATA)

#ifdef OTGW_PROCESS_SUMMARY
bool lastFrameIsSummary = false;

void requestSummary(bool ON) {
  if (! processData) return;
  if (ON) Serial.write("\r\nPS=1\r\n");
  else Serial.write("\r\nPS=0\r\n");
  Serial.flush();
}
#endif // OTGW_PROCESS_SUMMARY

void runProcessData(bool run) {
  processData = run;
  lastOTGWmsg1.nb = 0;
  lastOTGWsummary1.nb = 0;
}


// extrait eventuellement un message du buffer. Fin du msg : CR (\r : 0x0d) puis LF (\n 0x0a)
// puis le stocke
size_t processOTGWbuffer(char *buffer, size_t sizeOldBuffer, size_t serial_got) {
  int nbEOL = 0;
  int offsetEOL = -1;
  char *serialBuffer = buffer + sizeOldBuffer;             // start of last received data from Serial
  int sizeCompleteBuffer = sizeOldBuffer + serial_got;     // total size of buffer
  
  if (! processData)   return 0;
  if (serial_got == 0) return  sizeOldBuffer;              // no new data
 
  // search if CR-LF in last part of buffer
  for (size_t i = 0; i < serial_got; i++)
  {
    if (serialBuffer[i] == '\n') { 
      nbEOL++;
      offsetEOL = sizeOldBuffer + i;                       // offset of last LF in buffer
    }
  }

/*
  if (nbEOL >= 0)                  // for debugging
  {
    d_printf("%snbEOL = %d, offsetEOL = %d, sizeOldBuffer = %u, serial_got = %u, sizeCompleteBuffer = %d%s", EOL, nbEOL, offsetEOL, sizeOldBuffer, serial_got, sizeCompleteBuffer, EOL);
    printDebugHex("serialBuffer", serialBuffer, serial_got, false);
    printDebugHex("buffer", buffer, 20, false);
  }
*/

  if (nbEOL == 0) return sizeCompleteBuffer;                // no CR-LF

  size_t newBufferSize = sizeCompleteBuffer - offsetEOL - 1;

  if (nbEOL > 1) {                                             // more of one EOL. No treated in this algo
    recordErrors("WARN. processOTGWbuffer(). More of one EOF in OTGW msg");
    d_printf("processOTGWbuffer. Plus d'un CR-LF dans buffer. serial_got = %u%s", serial_got, EOL);
    // printDebugHex("buffer", buffer + sizeOldBuffer, serial_got, false);
    memmove(buffer, buffer + offsetEOL + 1, newBufferSize);    // skip all before last CR-LF
    return(newBufferSize);
  }  
  
  // here, nbEOL > 0 ; on a un CR-LF dans buffer. LF en position offsetEOL
  if (offsetEOL < 2) return 0;         // buffer = "\r" (0) or buffer = "\r\n" (1)

                // copy msg to lastOTGWmsg1.msg or to lastOTGWsummary1.msg
  size_t sizeLastOTGWmsg = offsetEOL - 1;                // size of OTGW msg

  if (lastFrameIsSummary) {                    // previous frame was 'PS: 1'
    lastFrameIsSummary = false;
    if (sizeLastOTGWmsg > 110) {               // we are sure it's a summary frame
      size_t len = std::min(sizeLastOTGWmsg, (size_t) OTGW_LAST_MSG_MAX_SIZE);
      memmove(lastOTGWsummary1.msg, buffer, len);
      lastOTGWsummary1.msg[len] = '\0';
      lastOTGWsummary1.nb++;
      requestSummary(false);                       // send 'PS=0'
    }
    return newBufferSize;
  }

  // here, it isn't a summary message

  size_t len = std::min(sizeLastOTGWmsg, (size_t) OTGW_COMMON_MSG_MAX_SIZE);
  memmove(lastOTGWmsg1.msg, buffer, len);
  lastOTGWmsg1.msg[len] = '\0';
  lastOTGWmsg1.nb++;

  memmove(buffer, buffer + offsetEOL + 1, newBufferSize);    // skip all before last CR-LF

  if (sizeLastOTGWmsg == 9) return newBufferSize;          // standard msg
  
  // d_printf("no standard : %u -> |%s|%s", sizeLastOTGWmsg, msg, EOL);
  // printDebugHex("", msg, sizeLastOTGWmsg + 2, false);




  else if (sizeLastOTGWmsg == 5) {
    if (strcmp(lastOTGWmsg1.msg, "PS: 1") == 0)  lastFrameIsSummary = true;
    if (strcmp(lastOTGWmsg1.msg, "PS: 0") == 0)  lastFrameIsSummary = false;
  }

  return newBufferSize;
}

#else  //OTGW_PROCESS_DATA


#endif  //OTGW_PROCESS_DATA
