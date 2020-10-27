# OTGW with nodemcu (esp8266)

## Description

Application 'arduino' pour un nodemcu ou autre matériel esp8266, relié à une gateway opentherm de [Nodoshop](https://www.nodo-shop.nl/en/opentherm-gateway/188-opentherm-gateway.html).
Cette gateway est issue de l'[OpenTherm Gateway](http://otgw.tclcode.com) (**OTGW**).

Elle permet de rendre disponible sur un réseau wifi, et en telnet, les informations séries issues de OTGW.

Cette application permet en outre : 
* plusieurs clients telnet simultanés en liaison avec l'interface série de OTGW
* un accès telnet (ESP commands) dédié à l'administration et au suivi du fonctionnement du nodemcu
* un accès telnet dédié au debugging à distance
* un accès http pour permettre d'interfacer une application domotique avec OTGW de manière simple
* une mise à jour OTA du firmware du nodemcu

Une part importante du code est issue du projet [ESP8266 OTGW](https://github.com/SenH/ESP8266-OTGW) de SenH

## Installation

* copier `config.example.h` en `config.h`, et configuer.
* compiler et flasher, via l'IDE Arduino ou l'IDE PlatformIO, en choisissant comme plate-forme de développement 'NodeMCU 1.0'.

### configuration
Dans la première partie de config.h,  il faut au minimum paramétrer les informations de connexion wifi :
* '#define wifi1_ssid' : le SSID de la connexion Wifi
* '#define wifi1_password' : le password associé à ce SSID

A noter qu'on peut paramétrer 3 SSID distincts.

### librairies nécessaires

Aucune, si l'IDE utilise la plate-forme de développement 'NodeMCU 1.0'.

### précaution avant flashage
Il faut, lors de l'opération de flashage, que les deux cavaliers de l'OTGW soient retirés : il ne faut pas que l'interface série de l'OTGW soit redirigée vers le nodemcu.\
Pour les mises à jour suivantes du firmware, privilégier la mise à jour OTA, qui ne présente pas cette contrainte.

## Usage

### bridge OTGW série vers telnet
C'est l'utilisation première ; par défaut :\
`telnet OTGW_IP 23`
* Le port TCP peut être modifié dans config.h : '`#define telnet_OTGW_port`'
* le nombre de sessions telnet simultanées peut être modifié dans config.h : '`#define telnet_OTGW_max_sessions`'

### administration ESP
Optionnel. Permet d'administrer le nodemcu via une session telnet\
`telnet OTGW_IP 24`\
L'option peut être désactivée en commentant dans config.h : '`#define USE_TELNET_ADMIN`'\
Le port TCP peut être modifié dans config.h : '`#define telnet_admin_port`'

Les commandes possibles :

Command | Usage
------- | -----
`$SYS` | Display ESP Arduino version info, uptime & last restart reason
`$MEM` | Display free memory & fragmentation
`$NET` | Display IP, Subnet, Gateway, DNS and MAC address
`$WIF` | display WiFi diagnostics
`$UPD` | Update ESP firmware 
`$RST ESP` | Reset ESP
`$RST OTGW` | Reset OTGW
`$HLP` | Display usage

A noter la commande '`$UPD`' : elle permet la mise à jour OTA (à distance) du firmware du nodemcu.\
L'URL du firmware est indiquée dans config.h : '`#define esp_update_url`'
### debugging
Optionnel. Permet d'avoir des infos de debugging, et d'agir sur le fonctionnement de l'analyse des données de l'OTGW\
L'option peut être désactivée en commentant dans config.h : '`#define USE_TELNET_DEBUG`'\
Le port TCP peut être modifié dans config.h : '`#define telnet_debug_port`'

Les commandes possibles :

Command | Usage
------- | -----
`start` | (re) démarrage de l'analyse des données provenant de l'OTGW
`stop` | arrêt de l'analyse des données provenant de l'OTGW
`dbg on` | activation des messages de debug
`dbg off` | arrêt des messages de debug
`msgs on` | affichage des messages OTGW décodés
`msgs off` | arrêt de l'affichage des messages OTGW
`errors` | affichage du log des informations ou erreurs importantes
