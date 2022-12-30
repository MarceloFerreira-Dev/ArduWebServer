#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1, 111);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

// Relay state and pin
String LEDState = "Off";
const int Led = 7;

// Client variables 
char linebuf[80];
int charcount=0;

void setup() { 
  // Relay module prepared 
  pinMode(Led, OUTPUT);
  digitalWrite(Led, HIGH);
  
  // Open serial communication at a baud rate of 9600
  Serial.begin(9600);
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

// Display dashboard page with on/off button for relay
// It also print Temperature in C and F
// client.println("");
void dashboardPage(EthernetClient &client) {
  client.println("<!DOCTYPE HTML><html lang='en'><html><head><title>GATE</title>");
  client.println("<meta charset='utf-8'>");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><body>");                                                             
  client.println("<h3>Gate control - <a href=\"/\">Refresh</a></h3>");
  // Generates buttons to control the relay
  client.println("<h4>LED ON - State: " + LEDState + "</h4>");
  // If relay is off, it shows the button to turn the output on          
  if(LEDState == "Off"){
    client.println("<a href=\"/led_on\"><button>ON</button></a>");
  }
  // If relay is on, it shows the button to turn the output off         
  else if(LEDState == "On"){
    client.println("<a href=\"/led_off\"><button>OFF</button></a>");                                                                    
  }
  client.println("</body></html>"); 
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    memset(linebuf,0,sizeof(linebuf));
    charcount=0;
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
       char c = client.read();
       //read char by char HTTP request
        linebuf[charcount]=c;
        if (charcount<sizeof(linebuf)-1) charcount++;
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          dashboardPage(client);
          break;
        }
        if (c == '\n') {
          if (strstr(linebuf,"GET /led_off") > 0){
            digitalWrite(Led, HIGH);
            LEDState = "Off";
          }
          else if (strstr(linebuf,"GET /led_on") > 0){
            digitalWrite(Led, LOW);
            LEDState = "On";
          }
          // you're starting a new line
          currentLineIsBlank = true;
          memset(linebuf,0,sizeof(linebuf));
          charcount=0;          
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}