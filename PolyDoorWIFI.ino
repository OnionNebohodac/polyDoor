//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include "config.h"
#include <HTTPClient.h>


// Create instances
MFRC522 mfrc522(SS_PIN, RST_PIN);

bool door_locked = true;
bool door_closed = false;
unsigned long door_open = 0;
unsigned long last_open = 0;
const char *ssid = "Polyhedra";
const char *password = "polyhedra3d";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 10000;

String GOOGLE_SCRIPT_ID = "AKfycbwJAEoSi6vcABko5iTD-Q3XUKeD-CPud2Lvb98-xz9LzPhTtnFN_52YxKD8uzWeE-o-";
String urlFinal;
int buttonCounter = 0;
String logTrigger = "";
String output4State = "off";
int rfidLastUpdate = RFIDUPDATETIME;
//bool rfidinit = false;
WiFiServer server(80);
String header;  //holds HTTP request
class Terminal {
public:
  Terminal() {
    source = 0;
  }
  int println(char *buff);
  int source;
};

int Terminal::println(char *buff) {
  if (source == CMD_SOURCE_SERIAL) {
    Serial.println(buff);
  }
  return 1;
}
Terminal *terminal;

void setup() {
  for (int i = 0; i < MEMBERS; i++) {
    MasterTag[i] = 0;
  }
  MasterTag[0] = new String("C1834E24");
 

  
  terminal = new Terminal();
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);

  pinMode(DOOR_C, OUTPUT);
  //PinMode(DOOR_NO, INPUT);
  pinMode(DOOR_NC, INPUT);

  Serial.begin(115200);

  Serial.println("The device started, now you can pair it with bluetooth!");

  // Initiating
  SPI.begin();         // SPI bus
  mfrc522.PCD_Init();  // MFRC522

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  static char cmd = CMD_NOOP;
  uint8_t source = CMD_SOURCE_SERIAL;

  if (Serial.available()) {
    cmd = Serial.read();
    source = CMD_SOURCE_SERIAL;
  } else if (
    digitalRead(PIN_BUTTON) == LOW) {
    cmd = CMD_OPEN;
    source = CMD_SOURCE_BUTTON;
    logTrigger = "button";
  }
  terminal->source = CMD_SOURCE_SERIAL;

  digitalWrite(DOOR_C, HIGH);
  delay(5);
  if ((door_locked == true) && (digitalRead(DOOR_NC) == LOW)) {
    door_closed = true;
  } else  //if (digitalRead(DOOR_NO) == LOW)
  {
    door_closed = false;
  }
  digitalWrite(DOOR_C, LOW);

  if ((cmd == '\r') || (cmd == '\n')) {
    cmd = CMD_NOOP;
  }
  if (getID()) {
    if ((query_access(tagID)) || ((DEBUG == true) && (!tagID.equals(String(""))))) {
      if (door_locked == true) {
        source = CMD_SOURCE_RFID;
        cmd = CMD_OPEN;
        terminal->println(" Access Granted!  ");
        logTrigger = "rfid";
        // You can write any code here like opening doors, switching on a relay, lighting up an LED, or anything else you can think of.
      } else {
        terminal->println(" Access Denied!  ");
      }
    }
  }
  //Serial.print(tagID);
  

  WiFiClient client = server.available();  // Listen for incoming clients
  if (client) {                            // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");                                             // print a message out in the serial port
    String currentLine = "";                                                   // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        Serial.write(c);         // print it out the serial monitor
        header += c;
        if (c == '\n') {  // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              cmd = CMD_OPEN;
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              cmd = CMD_CLOSE;
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html {font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button {background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".errorbar {background-color:#ff000; width:100%;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            if (door_locked == true) {
              output4State = "on";
            } else {
              output4State = "off";
            }
            // Display current state, and ON/OFF buttons for GPIO 26
            client.println("<p>GPIO 4 - State " + output4State + "</p>");
            // If the output26State is off, it displays the ON button
            if (door_locked == true) {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">OPEN DOOR</button></a></p>");
              if (door_closed == false) {
                client.println("<div class=\"errorbar\">THE DOOR IS NOT CLOSED!</div>");
              }
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">CLOSE DOOR</button></a></p>");
            }
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else {  // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  if (cmd != CMD_NOOP) {
    terminal->println(&cmd);
    switch (tolower(cmd)) {
      case CMD_STATUS:
        if (door_locked == false) {
          terminal->println("Open");
        } /*else if (door_closed == false) {
              terminal->println("Locked but not closed");
          } */
        else {
          terminal->println("Closed");
        }
        break;
      case CMD_OPEN:
        digitalWrite(PIN_RELAY, HIGH);
        last_open = millis();
        door_locked = false;
        terminal->println("Opened");
        break;
      case CMD_CLOSE:
        digitalWrite(PIN_RELAY, LOW);
        terminal->println("Closed");
        door_locked = true;
        break;
      case CMD_HELP:
        terminal->println("?\tHELP");
        terminal->println("x\tCLOSE");
        terminal->println("v\tOPEN");
        terminal->println("s\tSTATUS");
        break;
    }
    cmd = CMD_NOOP;
  }
  if (door_locked == false) {
    if (millis() - last_open > TIMEON) {
      digitalWrite(PIN_RELAY, LOW);
      door_locked = true;
    }
  }


    
  if (millis() - rfidLastUpdate > RFIDUPDATETIME) {
    Serial.print("Task1 running on core ");
    Serial.println(xPortGetCoreID()); 
    HTTPClient http;
  for (int i = 1; i < MEMBERS ; i++) {
    Serial.println("Ovde sam");
    int j = i+1;
    urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?sheet=rfidlist&read=a" + j;
    Serial.println(urlFinal);
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    String payload1 = String(http.getString()); 
    urlFinal = "";
    Serial.print("nemoj me zaboraviti ");
    Serial.print(payload1);
    if (payload1 == "") break;
    MasterTag[i] = &payload1;
    Serial.print("init rfid ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(*MasterTag[i]);  
  } 
    http.end();  
    rfidLastUpdate = millis();
    Serial.println("rfidinit true");
  }   
  

  
  if (!(logTrigger == "")){
    if (logTrigger == "button")  urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?sheet=rfidlog" + "&count=" + String(buttonCounter) + "&tagid=button%20pressed";
    else if (logTrigger == "rfid")  urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?sheet=rfidlog" + "&count=" + String(buttonCounter) + "&tagid=" + tagID;
    Serial.print("POST data to spreadsheet:");
    Serial.println(urlFinal);
    HTTPClient http;
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    String payload;
    if (httpCode > 0) {
      payload = http.getString();
      Serial.println("Payload: " + payload);
    }
    http.end();
    buttonCounter++;
    logTrigger = "";
    urlFinal = "";
  }
  tagID = "";   
delay(20);
}


boolean query_access(String tagID) {
  for (int i = 0; i < MEMBERS; i++) {
    if (MasterTag[i] == 0) return false;
    Serial.println(String("'") + tagID + "' <=> '" + *MasterTag[i] + "'");
    if (tagID.equals(*MasterTag[i])) {
      Serial.println("Gotta");
      return true;
    }
  }
  return false;
}

//Read new tag if available
boolean getID() {
  // Getting ready for Reading PICCs
  if (!mfrc522.PICC_IsNewCardPresent()) {  //If a new PICC placed to RFID reader continue
    return false;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {  //Since a PICC placed get Serial and continue
    return false;
  }
  tagID = "";
  for (uint8_t i = 0; i < 4; i++) {  // The MIFARE PICCs that we use have 4 byte UID
    //readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX));  // Adds the 4 bytes in a single String variable
  }
  //byte readbackblock[18];
  tagID.toUpperCase();
  Serial.println(tagID);
  mfrc522.PICC_HaltA();  // Stop reading
  return true;
}
/*
void rfidUpdate(void * para) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID()); 
    HTTPClient http;
  for (int i = 1; i < MEMBERS ; i++) {
    Serial.println("Ovde sam");
    int j = i+1;
    urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?sheet=rfidlist&read=a" + j;
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    */
    /*int httpCode = http.GET();
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);*/
    /*
    String payload1 = String(http.getString()); 
    urlFinal = "";
    http.end();
    if (payload1 == "") break;
    MasterTag[i] = &payload1;
    Serial.print("init rfid ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(*MasterTag[i]);  
  }  

}*/