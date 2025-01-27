#ifndef _DOORLOCK
#define _DOORLOCK
// Definitions
#define PIN_BUTTON 12
#define PIN_RELAY 4
#define TIMEON 5 * 1000
#define SENDFREQ 20 * 1000  //unused
//rfid
#define RST_PIN 0
#define SS_PIN 5
#define DEBUG false

#define DOOR_C 32
//#define DOOR_NO 27 //unused 
#define DOOR_NC 33

#define MISO 19
#define MOSI 23
#define SCK 18


/*
Vcc <-> 3V3 (or Vin(5V) depending on the module version)
RST (Reset) <-> D0
GND (Masse) <-> GND
MISO (Master Input Slave Output) <-> 19
MOSI (Master Output Slave Input) <-> 23
SCK (Serial Clock) <-> 18
SS/SDA (Slave select) <-> 5
*/
#define ON 1
#define OFF 0
// PROTOCOLS
#define WIFION ON
#define SSID "polyhedra"
#define PASSWD "polyhedra3d"

// COMMANDS
#define CMD_HELP '?'
#define CMD_OPEN 'v'
#define CMD_CLOSE 'x'
#define CMD_STATUS 's'
#define CMD_NOOP '\0'

#define CMD_SOURCE_BT 0
#define CMD_SOURCE_SERIAL 1
#define CMD_SOURCE_BUTTON 2
#define CMD_SOURCE_RFID 3
#define MEMBERS 20
#define RFIDUPDATETIME 10 * 60 * 1000

#define BOT_TOKEN "5853998721:AAHPN0mbDyA7oYJaMEQfWokrCppiX3iOGoM" //telegram bot
#define CHAT_ID_1 "5958442135" //KOSTA TELEGRAM
#define CHAT_ID_2 "5709761234" //MEIR TELEGRAM

#define TESTPIN1 26
#define TESTPIN2 27
//GLOBAL VARS
byte readCard[4];
String * MasterTag[MEMBERS] ;	

String tagID = "";

boolean getID();
boolean query_access(String tagID);
#endif
