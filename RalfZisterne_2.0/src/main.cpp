#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EMailSender.h>
#include <EEPROM.h>
//////eigene Libs
#include <EigHTML.h>
#include <klassen.h>
#include <Sensor.h>

const char* ssid="Paradieswerkstatt"; //"AndroidAPp";//"WLAN Ralf"
const char* password="k*H#96*c*Z#93";//"vjjb1674"; //"44727291469489115864"

WiFiServer server(80);///AsyncWebServer server(80); //Server instanz, Port?

String header;// Variable to store the HTTP request
unsigned long currentTime = millis();// Current time
unsigned long previousTime = 0; // Previous time
const long timeoutTime = 2000; // Define timeout time in milliseconds (example: 2000ms = 2s)

int j=0; //Sommer/Winterzeit
unsigned int letzteAbfrage =0; //für die Tägliche Abfrage

struct Fuellstand* Liste=NULL; //Liste erstellen

WiFiUDP ntpUDP; //Fuer die Package´s die von den Zeit Server zu holen sind
long utcOffsetInSeconds = 3600; // offset von UTF zeit (zur Witerzeit, zur Sommerzeit erhöhen auf 7200)
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds); //Adresse und Client für die Zeit desfinieren

EMailSender emailSend("ralfszisterne@gmail.com", "A#dCa3D-;yky"); //Email
const char* empfaenger = "felix.girke@web.de";
const char* empfaenger2="ralfszisterne@gmail.com"; //zum selbst emailen
EMailSender::EMailMessage messageVoll;
EMailSender::EMailMessage messageLeer;
EMailSender::EMailMessage messageAnfrage;

const int SpeicherPos=301;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: "); //IP Adresse ausgeben
  Serial.println(WiFi.localIP());

  EEPROM.begin(SpeicherPos+1); //Dauerspeicher reservieren (20*15+2 byte) 
  timeClient.begin();
  server.begin();
  //Emails Voll/Leer
  messageLeer.subject="Zisterne Voll";
  messageLeer.message="Die Zisterne ist über 50cm voll.";
  messageVoll.subject="Zisterne Leer";
  messageVoll.message="Die Zisterne ist nur noch unter 200cm voll";

  /*if(Liste==NULL){ //Keine Messwerte vorhanden  //geht noch nicht!!!!!
    Serial.println("hans");
    if(int(EEPROM.read(SpeicherPos))!=NULL){//check den EEPROM
      Serial.println("Klaus");
      getValueEEPROM(SpeicherPos, &Liste);//Einlesen
      KartenRechner(Liste);//geb den Eintägen noch die Liter
    } else{//sonst leere Karte erstellen
      Serial.println("gustav");
      erstellen(&Liste, 0, 0, 0, false, 0);
      EEPROM.write(SpeicherPos, 1);
      EEPROM.commit();
      
    }
  }*/
    erstellen(&Liste, 0, 0, 0, false, 0);
}

void loop() {
  if(Liste->Zaehler>1700){ //wenn der Speicher mit Instanzen voll ist Restarte
    ESP.reset();
  }
  timeClient.update();
  
  if(hour(timeClient.getEpochTime())==2 && timeClient.getEpochTime()>(letzteAbfrage+4000)){ //wenn es 2uhr ist und mind 4000s später als die letzte Automatische Abrage (>1h)
    letzteAbfrage=timeClient.getEpochTime();
    int F=Sensor();// Abfrage starten
    int L=Rechner(F);
    timeClient.update();
    unsigned int Unix=(timeClient.getEpochTime()+j); //Sommer/Winterzeit 
    erstellen(&Liste, F, L, Unix, true, SpeicherPos);
    messageAnfrage.subject="Anfrage Zisternenfüllstand"; //Tägliche email an sich selber damit das Google Konto nocht die Einstellungen ändert
    messageAnfrage.message=EmailAnfrage(Liste);
    EMailSender::Response resp=emailSend.send(empfaenger2, messageAnfrage);
    if (F<50){ //wenn fast leer
      EMailSender::Response resp=emailSend.send(empfaenger, messageLeer);
    } else if (F>200){ //wenn fast voll
      EMailSender::Response resp=emailSend.send(empfaenger, messageVoll);
    }
    
}

  WiFiClient client = server.available();
  if (client){
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected and not timed out
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then

        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            delay(1000);
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            if (header.indexOf("GET /6/on") >= 0) { //Sommer/Winterzeit button //Wenn Button aktiv -> Sommerzeit
               j=3600;
            }else if(header.indexOf("GET /6/off") >= 0){
              j=0;
            }
            if (header.indexOf("GET /5/on") >= 0) { //Start Button
              int F=Sensor();// Abfrage starten
              int L=Rechner(F);
              timeClient.update();
              unsigned int Unix=(timeClient.getEpochTime()+j); //Sommer/Winterzeit 
              erstellen(&Liste, F, L, Unix, true, SpeicherPos);
              /////////////////////////////////////////////////////////////testAusgabe
              int hh=0;
              Serial.println("test");
              for(hh=0;hh<=SpeicherPos;hh++){
                Serial.print(hh);
                Serial.print(" , ");
                Serial.println(EEPROM.read(hh));
              }
            } 
            if (header.indexOf("GET /7/on") >= 0){ //email Button
              messageAnfrage.subject="Anfrage Zisternenfüllstand";
              int F=Sensor();// Abfrage starten
              int L=Rechner(F);
              timeClient.update();
              unsigned int Unix=(timeClient.getEpochTime()+j); //Sommer/Winterzeit 
              erstellen(&Liste, F, L, Unix, true, SpeicherPos);
              messageAnfrage.message=EmailAnfrage(Liste);
              EMailSender::Response resp=emailSend.send(empfaenger, messageAnfrage);
            }             

            client.println(SendStandartseite(getDiagramWerte(Liste), getTabellenWerte(Liste),j));
          } else { // if you got a newline, then clear currentLine
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
}
/* ToDo
* dauerspeicher, umspeichern und löschen
* Auf Email reagieren
* mehrere WebSeiten
* Eventuell Gmail nochmal aktivieren
* Wenn eine min>10 vor null fällt weg
*/