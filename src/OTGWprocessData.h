#ifndef _OTGWPROCESSDATA_H_
#define _OTGWPROCESSDATA_H_

#include "config.h"

#define OTGW_LAST_MSG_MAX_SIZE 255
#define OTGW_COMMON_MSG_MAX_SIZE 40

#ifdef OTGW_PROCESS_DATA
  #define d_runProcessData(x)   runProcessData(x);
#else    // OTGW_PROCESS_DATA
  #define d_runProcessData(x)
#endif   // OTGW_PROCESS_DATA

struct lastOTGWmsg {
  char msg[OTGW_COMMON_MSG_MAX_SIZE + 1];
  unsigned long nb = 0;
};

struct lastOTGWsummary {
  char msg[OTGW_LAST_MSG_MAX_SIZE + 1];
  unsigned long nb = 0;
};

#define MAX_TYPE_ERRORS   4

struct lastErrors {
  const char * msg[MAX_TYPE_ERRORS];
  int nbErrorsByType[MAX_TYPE_ERRORS];
  int nb = 0;
};

extern void cleanErrors(void);
extern lastErrors * getErrors(void);
extern void recordErrors(const char *);

#ifdef OTGW_PROCESS_DATA

extern void OTGWprocessData_setup(void);
extern void runProcessData(bool);
extern size_t processOTGWbuffer(char *, size_t, size_t);
extern lastOTGWmsg * getOTGWmsg(void);

#ifdef OTGW_PROCESS_SUMMARY

extern void requestSummary(bool);
extern lastOTGWsummary * getSummary(void);

#endif // OTGW_PROCESS_SUMMARY

#endif  // OTGW_PROCESS_DATA



#endif  // _OTGWPROCESSDATA_H_