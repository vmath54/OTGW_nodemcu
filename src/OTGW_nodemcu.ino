//
// ESP8266 serial wifi bridge, for OTGW : Opentherm Gateway, with nodemcu
// See http://otgw.tclcode.com/
// See https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/WiFiTelnetToSerial/WiFiTelnetToSerial.ino
//
// This code come from https://github.com/SenH/ESP8266-OTGW

#include <Arduino.h>
#include "Config.h"
#include "httpServ.h"
#include "telnetAdmin.h"
#include "telnetDebug.h"
#include "OTGWprocessData.h"

#include <core_version.h>
#include <ESP8266WiFiMulti.h>       // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <Ticker.h>

#define baud_rate                 9600 
#define connectTimeoutMs                5000
#define wifi_connect_timeout            60000  // reboot ESP32 after this timer
#define wd2_interval                    1000

// 
// GLOBALS
// 

Ticker t_uptime;

bool start                        = true;

unsigned long uptime              = 0;        // Counting every second until 4294967295 = 130 years
unsigned long wifi_connect_ts     = 0;

char OTGWbuffer[OTGW_BUFFER_LENGTH + 1];      // buffer to read OTGW data via Serial
size_t sizeBuffer = 0;

WiFiServer telnet_otgw_server(telnet_OTGW_port);
WiFiClient telnet_otgw_clients[telnet_OTGW_max_sessions];

ESP8266WiFiMulti wifiMulti;

unsigned long wifi_connect_timer                        = 0;
unsigned long wd2_timer                                 = 0;

// 
// FUNCTIONS
// 

volatile static e_actions action = NONE;
volatile static e_actions secondAction = NONE;

void pushAction(e_actions newAction) {

  if (newAction == NONE) return;
  if (secondAction != NONE) {
    d_printf("WARN. pushAction. Too much actions. action = %d, secondAction =%d, newAction = %d%s", action, secondAction, newAction, EOL);
    recordErrors("WARN. pushAction. Too much actions");
    return;
  }
  if (action == NONE) action = newAction;
  else secondAction = newAction;
}

e_actions popAction() {
  e_actions returnAction;
  if (action == NONE) return NONE;
  returnAction = action;
  if (secondAction == NONE) {
    action = NONE;
  } else {
    action = secondAction;
    secondAction = NONE;
  }
  return returnAction;
}

void loop_restart(void) {
  start = true;
}

void uptime_inc() {                               // called every second
  uptime++;
#ifdef OTGW_PROCESS_SUMMARY
  if ((uptime % summary_period) == 0) {
    pushAction(SUMMARY_REQUEST);
  }
#endif  // OTGW_PROCESS_SUMMARY
}



int rssi_to_percent(float rssi) {
// https://stackoverflow.com/questions/15797920/
  int i = round(rssi);
  i = 2 * (i + 100);
  i = max(i, 0);
  return min(i, 100);
}

String get_net_info() {
  String s;
  s += F(" IP: ");
  s += WiFi.localIP().toString();
  s += EOL;
  s += F("SUB: ");
  s += WiFi.subnetMask().toString();
  s += EOL;
  s += F(" GW: ");
  s += WiFi.gatewayIP().toString();
  s += EOL;
  s += F("DNS: ");
  s += WiFi.dnsIP().toString();
  s += F(", ");
  s += WiFi.dnsIP(1).toString();
  s += EOL;
  s += F("MAC: ");
  s += WiFi.macAddress();
  
  return s;
}

 // See https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/WiFiMulti/WiFiMulti.ino 
void connect_to_wifi() {
  s_println("WiFi > Initialize");
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(wifi1_ssid, wifi1_password);
  if (wifi2_ssid[0] != '\0') wifiMulti.addAP(wifi2_ssid, wifi2_password);
  if (wifi3_ssid[0] != '\0') wifiMulti.addAP(wifi3_ssid, wifi3_password);


#ifdef STATIC_IP
  WiFi.config(host, gateway, netmask, dns1);
#endif
  
  // Wait for WIFI connection
  wifi_connect_timer = millis();
  s_println("WiFi > Connecting...");
  while (wifiMulti.run(connectTimeoutMs) != WL_CONNECTED) {
    s_println("WiFi > Connecting. Try new...");
    unsigned long now = millis();

    // Toggle LED
    digitalWrite(esp_led_pin, !digitalRead(esp_led_pin));
    delay(500);

    // Restart ESP after timeout
    if (now - wifi_connect_timer >= wifi_connect_timeout) {
      s_printf("WiFi > Failed connecting after %d seconds!%s", (wifi_connect_timeout / 1000), EOL);
      s_println(F("ESP > Restart now"));
      ESP.restart();
    }
  }
  wifi_connect_ts = uptime;
  s_printf("WiFi > Connected after %d seconds%s", (millis() - wifi_connect_timer) / 1000, EOL);
#ifdef SERIAL_PRINT
  WiFi.printDiag(Serial);
#endif
  s_printf("RSSI: %d dBm (%d%%)%s", WiFi.RSSI(), rssi_to_percent(WiFi.RSSI()), EOL);
  s_println(get_net_info());
}

void reset_otgw() {
  pinMode(otgw_reset_pin, OUTPUT);
  digitalWrite(otgw_reset_pin, LOW);
  delay(500);
  digitalWrite(otgw_reset_pin, HIGH);
  pinMode(otgw_reset_pin, INPUT_PULLUP);
  s_println("OTGW > Reset complete");
}


// 
// MAIN
// https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/WiFiTelnetToSerial/WiFiTelnetToSerial.ino
// 

// ################## setup ##########################

void setup(void) {
  // Increment uptime every second
  t_uptime.attach(1.0, uptime_inc);

  Serial.begin(baud_rate);
#ifdef SKETCH_VERSION
  s_printf("\nSketch:%s/", SKETCH_VERSION);
#endif
  s_println(ESP.getFullVersion());
  s_print("ESP > Restart reason: ");
  s_println(ESP.getResetReason());
  
  pinMode(esp_led_pin, OUTPUT);
  pinMode(node_led_pin, OUTPUT);
  // Set LEDS OFF
  digitalWrite(esp_led_pin, HIGH);
  digitalWrite(node_led_pin, HIGH);
  
#ifdef OTGW_PROCESS_DATA
  OTGWprocessData_setup();
#endif // OTGW_PROCESS_DATA

#ifdef RESET_OTGW_ON_BOOT
  reset_otgw();
#endif
  
  connect_to_wifi();
  telnet_otgw_server.begin();
  #ifdef USE_TELNET_ADMIN
  telnet_admin_server_setup();
  #endif
  #ifdef USE_TELNET_DEBUG
  telnet_debug_server_setup();
  #endif
  
  if (MDNS.begin("otgw")) {              // try 'ping otgw.local'
    s_println("MDNS responder started");
  }

#ifdef USE_HTTP_SERVER
  http_server_setup();
#endif

}

// ################## loop ##########################
void loop(void) {
  uint8_t i;
  size_t serial_got = 0;                        // bytes received from OTGW
  
  // Check WiFi status
  if(WiFi.status() != WL_CONNECTED) {
    connect_to_wifi();
    recordErrors("INFO. loop(). Wifi restart");
    start = true;                              // to flush Serial
  }

  // Set LEDS off
  digitalWrite(esp_led_pin, HIGH);
  digitalWrite(node_led_pin, HIGH);

#ifdef USE_HTTP_SERVER
  http_server_loop();
#endif

  MDNS.update();

#ifdef USE_TELNET_ADMIN
  telnet_admin_server_loop();
#endif

  if (start)               // flush Serial
  {
    start = false;
    Serial.write("\r\n");
    Serial.flush();
    delay(1000);
    size_t len = Serial.available();
    len = std::min((size_t) OTGW_BUFFER_LENGTH, len);
    Serial.readBytes(OTGWbuffer, len);
    Serial.readBytesUntil('\n', OTGWbuffer, OTGW_BUFFER_LENGTH);  // wait until CR-LF
    sizeBuffer = 0;
    d_printf("%s ###########loop start ##########%s", EOL, EOL);
  }

  // Get new OTGW clients
  if (telnet_otgw_server.hasClient()) {
    for (i = 0; i < telnet_OTGW_max_sessions; i++) {
      // Find free spot
      if (!telnet_otgw_clients[i]) {
        telnet_otgw_clients[i] = telnet_otgw_server.available();
        s_print("SRV > New client: ");
        s_println(i);
        break;
      }
    }
    // No free spot so reject
    if (i == telnet_OTGW_max_sessions) {
      WiFiClient a_client = telnet_otgw_server.available();
      a_client.printf_P(ERR_SERVER_BUSY, telnet_OTGW_max_sessions, EOL);
      s_println("SRV > Max clients exceeded. Rejecting!");
    }
  }
  
  // OTGW Telnet -> Serial
  for (i = 0; i < telnet_OTGW_max_sessions; i++) {
    while (telnet_otgw_clients[i].available() && Serial.availableForWrite() > 0) {
      if (telnet_otgw_clients[i].available()) {
        Serial.write(telnet_otgw_clients[i].read());
        Serial.flush(); // Wait for transmit to finish
        digitalWrite(node_led_pin, !digitalRead(node_led_pin));
        delay(1); // Increases LED ON time
      }
    }
  }

  // OTGW Serial -> Telnet
       
  size_t maxToTcp = 0;         // maximum acceptable size calculation for telnet sessions
  int nb_telnet_sessions = 0;  // number of telnet sessions in progress
  for (int i = 0; i < telnet_OTGW_max_sessions; i++) {
    // Determine maximum output size "fair TCP use"
    if (telnet_otgw_clients[i]) {
      nb_telnet_sessions++;
      size_t afw = telnet_otgw_clients[i].availableForWrite();
      if (afw) {
        if (!maxToTcp) {
          maxToTcp = afw;
        } else {
          maxToTcp = std::min(maxToTcp, afw);
        }
      } else {
        // warn but ignore congested clients
        s_println("SRV > One client is congested");
      }
    }
  }

  size_t len;          // number bytes to read
  if (nb_telnet_sessions == 0) {
    len = std::min((size_t) Serial.available(), OTGW_BUFFER_LENGTH - sizeBuffer);
  }
  else {
    len = std::min((size_t) Serial.available(), maxToTcp);
    len = std::min(len, (size_t) OTGW_BUFFER_LENGTH - sizeBuffer);
  }
    
  if (len) {          // bytes available on Serial. Concat with OTGWbuffer
    serial_got = Serial.readBytes(OTGWbuffer + sizeBuffer, len);

    // push UART data to all connected telnet clients
    for (int i = 0; i < telnet_OTGW_max_sessions; i++) {
      // if client.availableForWrite() was 0 (congested)
      // and increased since then,
      // ensure write space is sufficient:
      if (telnet_otgw_clients[i].availableForWrite() >= serial_got) {
        digitalWrite(esp_led_pin, !digitalRead(esp_led_pin));
        size_t tcp_sent = telnet_otgw_clients[i].write(OTGWbuffer + sizeBuffer, serial_got);
        if (tcp_sent != len) {
          recordErrors("WARN. loop(). WiFi > len mismatch");
          s_printf("WiFi > len mismatch: available:%zd serial-read:%zd tcp-write:%zd%s", 
                          len, serial_got, tcp_sent, EOL);
          d_printf("WiFi > len mismatch: available:%zd serial-read:%zd tcp-write:%zd%s", 
                          len, serial_got, tcp_sent, EOL);
        }
        delay(1); // Increases LED ON time
      }
    }
  }

#ifdef OTGW_PROCESS_DATA
  if (serial_got > 0) {
    sizeBuffer = processOTGWbuffer(OTGWbuffer, sizeBuffer, serial_got);  // extract a message if CR-LF
  }
  if (sizeBuffer >= OTGW_BUFFER_LENGTH) {
    sizeBuffer = 0;
    d_printf("%sloop(). ERROR. sizeBuffer = %u%s", EOL, sizeBuffer, EOL);
    recordErrors("ERROR. loop(). sizeBuffer too big");
  }
#else
  sizeBuffer = 0;
#endif

if (serial_got == 0) {                 // we run action when no Derail data available
  e_actions action = popAction();
  switch (action) {
    case NONE : break;
    case SUMMARY_REQUEST : requestSummary(true); break;
  }
}

#ifdef USE_TELNET_DEBUG  
  telnet_debug_server_loop();
#endif
}
