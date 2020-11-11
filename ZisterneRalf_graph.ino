// Load Wi-Fi library
#include <ESP8266WiFi.h> //WIFI-Lib
#include <SR04.h> //Ultarschallsensor
#include <NTPClient.h> 
#include <WiFiUdp.h>
#include <TimeLib.h>

// Replace with your network credentials
const char* ssid     = "Paradieswerkstatt"; //"WLAN Ralf"
const char* password = "k*H#96*c*Z#93"; //"44727291469489115864"

WiFiUDP ntpUDP; //Fuer die Package´s die von den Zeit Server zu holen sind
long utcOffsetInSeconds = 3600; // offset von UTF zeit (zur Witerzeit, zur Sommerzeit erhöhen auf 7200)
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds); //Adresse und Client für die Zeit desfinieren

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Assign output variables to GPIO pins
String s="";
const int historie=50; //Wieviel Werte sollen gespeichert werden
int a[historie]; //Speicherplatz für die Abstände
long long t[historie]; //Speicherplatz für die Zeitpunkte
int Trig=5; //Entspricht D1
int Echo=4; //Entspricht D2
int i=0; //zaehler Schleife ausgabe
int k=0; //zaehler einmal taegliche abfrage
int j=0; //offset Sommer/Winterzeit
SR04 sr04= SR04(Trig, Echo);

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Connect to Wi-Fi network with SSID and password
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
  timeClient.begin();
}
////////////////////////////////////////////////////Sensor
int Sensor(){
  int A=sr04.Distance(); //Messe dreimal hintereinander um Feler zu minimiern
  int B=sr04.Distance();
  int C=sr04.Distance();
  int D=(A+B+C)/3; //Durchschnitt der 3 Werte ermitteln
  Serial.print(D); Serial.println("cm");
  int F=240-D; //Die Höhe des Sensors (240cm) minus den Messwert ergibt den Füllstand
  return F;
  }
////////////////////////////////////////////////////////////Rechner
int Rechner(int a){
   float k=0.0; //definition rueckgage variable als float zum max 1. liter rundungsfehler zu haben
   float f=(float)a/10; //Fuelstand in dezimeter und in als float
   if(a<200){
    k=22*(100*acos(1-(f/10))-(10-f)*sqrt((20*f)-(f*f))); //macht aus der Füllhöhe die Liter (liegender Zylinder) Quelle:https://www.scalesoft.de/math/tank/tank.html
   }else{
    k=6283+55.416*(f-20); //Uber dem Zylinder
   }
   return (int)k; //rueckgabe der wertes mit typecast zu int
  }
  ////////////////////////////////////////////////////////////////Umspeichern
void Umspeichern(int* a, long long* t){ //Speicher die Daten im Array um das nur die neueste Gespeichert sind
  for(i=(historie-1);i>=0;i--){ //von hinten nimm das 2. letzte legs an letzte Pos. usw.
    a[i]=a[i-1];
    t[i]=t[i-1];
  }
  return;
}
  ////////////////////////////////////////////////////////////////loop
void loop(){
  timeClient.update(); 
  if (hour(timeClient.getEpochTime()+j)==2 & k==0){//einmal Täglich abrufen um 12:00Uhr
    Umspeichern(&a[0], &t[0]); 
    a[0]=Sensor();
    timeClient.update();
    t[0]=(timeClient.getEpochTime()+j);
    k++;
  }
  if(hour(timeClient.getEpochTime()+j)==3){ //Zaehler für einmal tägliches Abrufen zurücksetzten
    k==0;
  }
  
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
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
            if (header.indexOf("GET /6/on") >= 0) { //Wenn Button aktiv -> Sommerzeit
               j=3600;
            }else if(header.indexOf("GET /6/off") >= 0){
              j=0;
            }
            // "Wartet" auf den Aufruf der ip/5/on Seite
            if (header.indexOf("GET /5/on") >= 0) {
              Umspeichern(&a[0], &t[0]);
              a[0]=Sensor(); //Speichert den Abstand
              timeClient.update();
              t[0]=(timeClient.getEpochTime()+j); //Speichert die dazugehörige Zeit (UNIX) evtl. plus 1h fuer Sommerzeit
            } 
            //Start HTML Seite, Kommentarte sind dort
            client.println("<!DOCTYPE html><html lang='de'>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta name='author' content='Felix Girke'>");
            client.println("<meta name='viewport' content='width=device-width, inital-scale=1.0'>");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // Graph (javascript) von https://developers-dot-devsite-v2-prod.appspot.com/chart/interactive/docs/gallery/linechart.html#overview
            client.println("<script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script><script type='text/javascript'>google.charts.load('current', {'packages':['corechart']});google.charts.setOnLoadCallback(drawChart)");
            client.println("function drawChart(){var data = google.visualization.arrayToDataTable([['Datum', 'Liter']");
             for(i=0; i<historie; i++){ //Mach tabelleneinträge für alle einträge (histoire)
            if(t[i]==0){ //Wenn kein wert mehr drin steht, brich ab
              break;
            }
            client.print(",['");
            if (day(t[i])<10){
              client.print("0");
            }
            client.print(day(t[i]));
            client.print(".");
            if(month(t[i])<10){
              client.print("0");
            }
            client.print(month(t[i]));
            client.print(".");
            client.print(year(t[i]));
            client.print("',");
            client.print(Rechner(a[i]));
            client.println("]");
            //,['19.09.2020',200],['20.09.2020',200],['21.09.2020',660],['22.09.2020',400],['23.09.2020',199]
             }
            client.println("]);var options={curveType:'none',legend:'none',");
            client.println("};var chart = new google.visualization.LineChart(document.getElementById('curve_chart'));chart.draw(data, options);}</script>");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button {background-color: #00008B; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println("table, th, td  {border: 1px solid black; border-collapse: collapse;}</style> <title>Ralf's Zisternen Fuellstand</title></head>");
            
            // Web Page Body
            client.println("<body><h1>Ralf's Zisternen Fuellstand</h1>");
            if (j==3600) {
              client.println("<p>Die Uhrzeit wird aktuell in Sommerzeit angezeigt</p>");
            }else{
             client.println("<p>Die Uhrzeit wird aktuell in Winterzeit angezeigt</p>");
            }
            client.println("<p>neue Abfrage:</p>");
            client.println("<p><a href=\"/5/on\"><button class=\"button\">Start</button></a><a href=\'/'><button class=\"button\" style='background-color:#228B22'>aktualisieren</button></a></p>");
            if (j==3600) {
              client.println("<p><a href=\"/6/off\"><button class=\"button\" style='background-color:#3399ff'>Winterzeit aktivieren</button></a></p>");
            }else{
             client.println("<p><a href=\"/6/on\"><button class=\"button\" style='background-color:#ffb84d'>Sommerzeit aktivieren</button></a></p>"); 
            }
            client.print("<p>Der letzte gemessene Fuellstand sind<b> ");
            client.print(a[0]);
            client.println("cm</b></p>");
            client.print("<p>Dies entspricht ungefaehr<b> ");
            client.print(Rechner(a[0]));
            client.println("Liter</b></p><div style=' width: auto; height: 500px' id='curve_chart'></div> <br><table style='width:100%'>");
            client.println("<tr><th style='border-top:hidden;border-bottom:hidden;'></th><th style='width:150px'>Datum</th><th style='width:100px'>Fuellhoehe</th><th style='width:100px'>Liter</th><th style='border-top:hidden;border-bottom:hidden;border-right:hidden;'></th></tr>");
           for(i=0; i<historie; i++){ //Mach tabelleneinträge für alle einträge (histoire)
            if(t[i]==0){ //Wenn kein wert mehr drin steht, brich ab
              break;
            }
            client.print("<tr><td style='border-top:hidden;border-bottom:hidden;border-left:hidden;'> </td><td>");
            if (day(t[i])<10){
              client.print("0");
            }
            client.print(day(t[i]));
            client.print(".");
            if(month(t[i])<10){
              client.print("0");
            }
            client.print(month(t[i]));
            client.print(".");
            client.print(year(t[i]));
            client.print(" ");
            if(hour(t[i])<10){
              client.print("0");
            }
            client.print(hour(t[i]));
            client.print(":");
            if(minute(t[i])<10){
              client.print("0");
            }
            client.print(minute(t[i]));
            client.print("</td><td> ");
            client.print(a[i]);
            client.println("cm</td><td>");
            client.print(Rechner(a[i]));
            client.println("l</td><td style='border-top:hidden;border-bottom:hidden;border-right:hidden;'> </td></tr>");
           }
           
            client.println("</table><p style='text-align:right'>Die aktuelle Version 1.0 des Quellcodes gibt es <a href='https://github.com/Feeee23/RalfFuelstand' target='_blank' >hier</a></p>");
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
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

/* ToDo:
 * Diagram zeichnen
 * stecker
 * verteilerdose
 * historie verlängern?
 * Bilder
 */
