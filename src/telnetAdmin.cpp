#include <Arduino.h>
#include "ESP8266HTTPClient.h"
#include "ESP8266httpUpdate.h"

#include "config.h"
#include "telnetAdmin.h"


#if defined(USE_TELNET_ADMIN)

// ################### internal variables ############################

WiFiServer telnet_admin_server(telnet_admin_port);
WiFiClient telnet_admin_client;


// ################### internal functions ############################


// See https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266httpUpdate/examples/httpUpdate/httpUpdate.ino
void do_http_update(WiFiClient client) {
  WiFiClient OTAclient;
  ESPhttpUpdate.setLedPin(node_led_pin, LOW);
  t_httpUpdate_return ret = ESPhttpUpdate.update(OTAclient, esp_update_url);
  
  switch (ret) {
    case HTTP_UPDATE_FAILED:
    client.printf("HTTP update error %d, %s%s", ESPhttpUpdate.getLastError(), 
      ESPhttpUpdate.getLastErrorString().c_str(), EOL);
    break;

    case HTTP_UPDATE_NO_UPDATES:
    client.println("HTTP no update available");
    break;

    case HTTP_UPDATE_OK:
    client.println("HTTP update OK");
    break;
  }
}

void parse_admin_cmd(WiFiClient client) {
  String cmd;
  cmd.reserve(16);

  client.setTimeout(1); // Max waiting time for readStringUntil()
  cmd = client.readStringUntil('\r');
  client.readStringUntil('\n'); // Discard LF

  if (cmd.length() < 4) {
    client.println(ADMIN_USAGE);
    return;
  }
  
  if (cmd.equals("$SYS")) {
#ifdef SKETCH_VERSION
    client.printf_P(PSTR("Sketch:%s/"), SKETCH_VERSION);
#endif
    client.println(ESP.getFullVersion());
    client.printf("    ESP uptime: %lu seconds%s", uptime, EOL);
    client.printf("   WiFi uptime: %lu seconds%s", (uptime - wifi_connect_ts), EOL);
    client.printf("Restart reason: %s%s", ESP.getResetReason().c_str(), EOL);
  } else if (cmd.equals("$MEM")) {
    client.printf("Free: %d bytes Fragmentation: %d%%%s", ESP.getFreeHeap(), ESP.getHeapFragmentation(), EOL);
  } else if (cmd.equals("$NET")) {
    client.println(get_net_info());
  } else if (cmd.equals("$WIF")) {
    WiFi.printDiag(client);
    client.printf("RSSI: %d dBm (%d%%)%s", WiFi.RSSI(), rssi_to_percent(WiFi.RSSI()), EOL);
  } else if (cmd.equals("$UPD")) {
    client.printf("Update ESP via %s%s", esp_update_url, EOL);
    do_http_update(client);
  } else if (cmd.equals("$RST ALL")) {
    client.println("OTGW & ESP > Restart");
    reset_otgw();
    ESP.restart();
  } else if (cmd.equals("$RST ESP")) {
    client.println("ESP > Restart");
    ESP.restart();
  } else if (cmd.equals("$RST OTGW")) {
    client.println("OTGW > Reset");
    reset_otgw();
    client.println("OTGW > Reset complete");
  } else if (cmd.equals("$EXT")) {
    client.println("Goodbye...");
    client.stop();
  } else if (cmd.equals("$VER")) {
    client.printf("Version : %s%s", VERSION, EOL);
  } else if (cmd.equals("$HLP")) {
    client.println(ADMIN_USAGE);
  } else {
    client.println(ADMIN_USAGE);
  }
}


// ################### exported functions ############################

void telnet_admin_server_setup(void) {
  telnet_admin_server.begin();
}

void telnet_admin_server_loop(void) {
      // Get new admin client
  if (telnet_admin_server.hasClient()) {
    if (!telnet_admin_client) {
      telnet_admin_client = telnet_admin_server.available();
      s_println("SRV > New ESP client");
    }
    else { 
      WiFiClient a_client = telnet_admin_server.available();
      a_client.printf_P(ERR_SERVER_BUSY, 1, EOL);
      s_println("SRV > Max clients ESP exceeded. Rejecting!");
    }
  }
  
  // admin commands
  if (telnet_admin_client.available()) {
    parse_admin_cmd(telnet_admin_client);
  }

}

#else  // USE_TELNET_ADMIN

#endif  // USE_TELNET_ADMIN