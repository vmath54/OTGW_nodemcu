<?php
/* 
  ------------ interrogation d'une gateway opentherm, via un nodemcu et le sketch OTGW_nodemcu : 
         recuperation d'un summary  OTGW(PS=1), décodage et transmission a jeedom ----------------

  fonctionne dans l'environnement jeedom. Doit etre deposé (par exemple) dans /var/www/html/plugins/script/data/OTGW
  
  Peut être exécuté en ligne de commande (php OTGWsummary.php), ou via un navigateur pour essai
  ex : http://<IP jeedom>/plugins/script/data/OTGW/OTGWsummary.php
  Si via un navigateur, Il faut modifier le fichier  /var/www/html/plugins/script/data/.htaccess pour autoriser le réseau local à accéder au script.
  exemple d'ajout en fin de fichier : "Allow from 192.168.0"

  Coté jeedom, il faut 
    . créer un virtuel 'OTGW', comprenant autant de commandes que d'infos à mémoriser. Les infos sont de type binaire pour 'flame', numérique pour les autres ; on ne saisi pas de valeur ni de parametre.
	. créer un script (plugin script), avec une seule commande qui execute /var/www/html/plugins/script/data/OTGW/OTGWsummary.php
	  Ce script est en auto-actualisation (cron), t n'a pas besoin d'etre visible

  Ce programme php interroge la passerelle opentherm gateway (OTGW) à l'URL http://$OTGWipAddress/summary pour récupérer les infos
  de sommaire (summary) issues de OTGW (commande PS=1), puis 'pousse' ces valeurs vers les commandes du virtuel
  Les infos disponibles dans le summary sont décrites à http://otgw.tclcode.com/firmware.html#configuration 
  Exemple de message summary :  00000001/00001010,14.80,00000011/00000011,100.00,8/53,20.50,0.00,0.00,21.05,24.73,0.00,11.80,0.00,55/40,40/20,20.00,40.00,4064,5224,176,0,1942,2566,0,0
  
  Dans ce programme, il faut adapter :
    . $OTGWipAddress : l'IP de la gateway OTGW
	. $virtualID : l'ID du virtuel jeedom. Dispo dans 'configuration avancée' du virtuel
	. $virtualCMDs : Les infos que l'on désire voir remonter au virtuel. Il faut préciser l'ID des commandes correspondantes du virtuel
	. $summaryParameters : a modifier si on désire remonter d'autres infos. les clé de ce tableau doivent correspondre aux clés 
	    du tableau $virtualCMDs
	
*/

$OTGWipAddress = "192.168.x.y";
$OTGW_summary_URL = "http://$OTGWipAddress/summary";

$debug = false;

$virtualID = 31;			// a renseigner avec l'ID du virtuel OTGW, dans jeedom

$virtualCMDs = array(       // ID des commandes du virtuel OTGW
  "outsideTemperature"          => 218,
  "roomTemperature"             => 219,
  "boilerTemperature"           => 220,
  "relativeModulation"			=> 222,
  "controlSetPoint"				=> 221,
  "flame"                       => 225
);

require_once dirname(__FILE__) . './../../../../core/php/core.inc.php';

setlocale(LC_TIME,"fr_FR.utf8");

$virtual = eqLogic::byId($virtualID);
list($summary, $nbMsg) = getOTGWsummaryMsg($OTGW_summary_URL);   // recuperation du summary de OTGW

$nbMsg = intVal($nbMsg);
if ($debug) echo "summary = \"$summary\"<br/>\n";
if ($debug) echo "nbMsg = \"$nbMsg\"<br/>\n";
if ($nbMsg < 3) {
  if ($debug) echo "erreur, ou nbMsg < 3<br/>\n";
  exit();
}
$summaryArray = explode(",", $summary);
//var_dump($summaryArray);
if (count($summaryArray) != 25) {
  if ($debug) echo "erreur. Summary contient " . count($summaryArray) . " elements<br/>\n";
  exit();
}
// on recupere les elements qui semblent importants
$summaryParameters['status'] = $summaryArray[0];
$summaryParameters['controlSetPoint'] = $summaryArray[1];
$summaryParameters['relativeModulation'] = $summaryArray[6];
$summaryParameters['roomTemperature'] = $summaryArray[8];
$summaryParameters['boilerTemperature'] = $summaryArray[9];
$summaryParameters['outsideTemperature'] = $summaryArray[11];
$summaryParameters['returnWaterTemperature'] = $summaryArray[12];    // pas value
$summaryParameters['flame'] = getFlag($summaryParameters['status'], 12);   // 13eme flag en partant de la droite. 4eme de la gauche

//if ($debug) var_dump($summaryParameters);

foreach ($virtualCMDs as $key => $value) {
  $cmd = cmd::byId($value);
  $virtual->checkAndUpdateCmd($cmd, $summaryParameters[$key]);
  if ( $debug ) echo "$value - $key   : " . $summaryParameters[$key] . "<br/>\n";
}

exit();


// decode un message du genre : '00000001/00001010' ; retourne la valeur binaire en partant de la gauche. Origine = 0.
//                             dans cet exemple : offset = 12 => 1 (13eme flag)
function getFlag($msg, $offset) {
  $msg = str_replace("/", "", $msg);
  return $msg[$offset];
}

function getOTGWsummaryMsg($url) {
  $curl = curl_init();
  curl_setopt_array($curl, array(CURLOPT_URL => $url, CURLOPT_RETURNTRANSFER => true, CURLOPT_TIMEOUT => 5));
  $response = curl_exec($curl);
  //echo "response = "; var_dump($response); echo "<br/>\n";
  curl_close($curl);
  return (explode("\r\n", $response));	
}

function exitOnError(){
//  scenario::removeData('Viessmann_token');
//  scenario::removeData('Viessmann_tokenExpires');
  exit();
}

