// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <SR04.h> //Ultarschallsensor
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

// Replace with your network credentials
const char* ssid     = "HansGustav38.2"; //"WLAN Ralf"
const char* password = "Frieder17"; //"44727291469489115864"

WiFiUDP ntpUDP;
long utcOffsetInSeconds = 7200;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Assign output variables to GPIO pins
const int testpin = 15;
String s="";
const int historie=50;
int a[historie];
long long t[historie];
int Trig=5; //Entspricht D1
int Echo=4; //Entspricht D2
int i=0;
int k=0;
SR04 sr04= SR04(Trig, Echo);


// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(testpin, OUTPUT);
  // Set outputs to LOW
  digitalWrite(testpin, LOW);

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
  int F=D; //Die Höhe des Sensors (190cm) minus den Messwert ergibt den Füllstand
  return F;
  }
////////////////////////////////////////////////////////////Rechner
int Rechner(int a){
   int k=2*a; //macht aus der Füllhöhe die Liter
  return k;
  }
  ////////////////////////////////////////////////////////////////Umspeichern
void Umspeichern(int* a, long long* t){ //Speicher die Daten um das nur die neueste Gespeichert sind
  for(i=(historie-1);i>=0;i--){
    a[i]=a[i-1];
    t[i]=t[i-1];
  }
  return;
}
  ////////////////////////////////////////////////////////////////loop
void loop(){
  timeClient.update(); //einmal Täglich abrufen um 12:00Uhr
  if (hour(timeClient.getEpochTime())==12 & k==0){//3:14:15
    Umspeichern(&a[0], &t[0]);
    a[0]=Sensor();
    timeClient.update();
    t[0]=timeClient.getEpochTime();
    k++;
  }
  if(hour(timeClient.getEpochTime())==21){
    k==0;
  }
  
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
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
            // turns the GPIOs on and off
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              //digitalWrite(testpin, HIGH);
              Umspeichern(&a[0], &t[0]);
              a[0]=Sensor();
              timeClient.update();
              t[0]=timeClient.getEpochTime();
              //digitalWrite(testpin, LOW);
            } 
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button {background-color: #195B6A; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            //client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println("table, th, td  {border: 1px solid black; border-collapse: collapse;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Ralfs Zisternen Fuellstand</h1>");
            client.println("<p>neue Abfrage:</p>");
            client.println("<p><a href=\"/5/on\"><button class=\"button\">Start</button></a><a href=\'/'><button class=\"button\">aktualisieren</button></a></p>");
            client.print("<p>Der letzte gemessene Fuellstand sind<b> ");
            client.print(a[0]);
            client.println("cm</b></p>");
            client.print("<p>Dies entspricht ungefaehr<b> ");
            client.print(Rechner(a[0]));
            client.println("Liter</b></p> <br><table style='width:100%'>");
            client.println("<tr><th style='border-top:hidden;border-bottom:hidden;'></th><th style='width:150px'>Datum</th><th style='width:100px'>Fuellhoehe</th><th style='width:100px'>Liter</th><th style='border-top:hidden;border-bottom:hidden;border-right:hidden;'></th></tr>");
           for(i=0; i<historie; i++){
            if(t[i]==0){
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

/* ToDo
 * rechner richten
 * Diagram zeichnen
 * stecker
 * verteilerdose
 * historie verlängern?
 * Bilder
 * sommerzeit
 */
