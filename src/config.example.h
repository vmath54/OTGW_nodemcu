#ifndef _OTGWCONFIG_H_
#define _OTGWCONFIG_H_

#include "Ethernet.h"

#define VERSION           "1.0.1"

#define USE_WATCHDOG                // for a user watchdog
#define WATCHDOG_DELAY    70        // delay, in seconds, for user watchdog

#define USE_TELNET_ADMIN
#define USE_TELNET_DEBUG
#define USE_HTTP_SERVER

//#define SERIAL_PRINT               // for debug via Serial. NOT USE if nodemcu is connected with OTGW via Serial
#define TELNET_DEBUG_PRINT           // to print via telnet. Require USE_TELNET_DEBUG
#define RESET_OTGW_ON_BOOT
// #define STATIC_IP
#define OTGW_PROCESS_DATA           // to decode OTGW frames
#define OTGW_PROCESS_DATA_ON_BOOT   // Require OTGW_PROCESS_DATA. if defined, process data start on boot
#define OTGW_PROCESS_SUMMARY        // Require OTGW_PROCESS_DATA. if defined, run command 'PS=1' every summary_period seconds

#define OTGW_BUFFER_LENGTH  255     // buffer to read Serial. 

#define summary_period            55
#define telnet_OTGW_port          23
#define telnet_OTGW_max_sessions  2
#define telnet_admin_port         24
#define telnet_debug_port         25
#define http_server_port          80

# define wifi1_ssid           "xxx"
# define wifi1_password       "xxx"
# define wifi2_ssid           ""
# define wifi2_password       ""
# define wifi3_ssid           ""
# define wifi3_password       ""

#define esp_update_url        "http://yourURL.org/opentherm/OTGW.bin"

#ifdef STATIC_IP
  IPAddress host(192, 168, 0, 55);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress netmask(255, 255, 255, 0);
  IPAddress dns1(192, 168, 0, 1);
#endif

// ###################### common defines and definitions #########################
//                      don't change the following lines

#ifndef OTGW_PROCESS_DATA
#undef OTGW_PROCESS_DATA_ON_BOOT
#undef OTGW_PROCESS_SUMMARY
#endif  // OTGW_PROCESS_DATA

#ifndef USE_TELNET_DEBUG
#undef TELNET_DEBUG_PRINT
#endif  // USE_TELNET_DEBUG

#ifdef SERIAL_PRINT
  #define s_print(x) Serial.print(x)
  #define s_println(x) Serial.println(x)
  #define s_printf(format,...) Serial.printf(format, __VA_ARGS__)
#else
  #define s_print(x)
  #define s_println(x)
  #define s_printf(...)
#endif

#define esp_led_pin               2
#define node_led_pin              16
#define otgw_reset_pin            14
#define wd_i2c_address            38

#define EOL                       "\r\n"
#define ERR_SERVER_BUSY           "Server is busy with %d active connections%s"

enum e_actions { NONE, SUMMARY_REQUEST };

extern void pushAction(e_actions);
extern e_actions popAction(void);
extern void loop_restart(void);
extern void reset_otgw(void);
extern int rssi_to_percent(float);
extern String get_net_info();
extern unsigned long uptime;
extern unsigned long wifi_connect_ts;

#endif  // _OTGWCONFIG_H_