#include <SimpleTimer.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
//#include <BlynkSimpleEsp8266.h>

//#include "DHT.h"

#include "auth.h"

// Initialize DHT sensor.
//DHT dht(D4, DHT22);

ESP8266WebServer server ( 80 );

const int LED = BUILTIN_LED;

SimpleTimer timer;

const char *myhostname = "bubbles";

const int history_length = 1024;
const int interval = 25;

int history_time[history_length];
char history_value[history_length];
int history_write_position = 0;
int history_read_position = 0;
//int current_time = 0;

void handleRoot() {
//  Serial.println ( "Root request" );
  digitalWrite(LED, 1);
  String s = "";
  if (history_read_position != history_write_position) {
    for (int i = history_read_position; i != history_write_position; i = (i+1) % history_length) {
      s += history_time[i];
      s += ",";
      s += (int)history_value[i];
      s += "\n";
    }
    history_read_position = history_write_position;
  }
  server.send(200, "text/csv", s);
  digitalWrite(LED, 0);
}

void handleNotFound() {
  digitalWrite(LED, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED, 0);
}

void sample() {
  digitalWrite(LED, 1);
//    current_time += 1;
  int sensorValue = analogRead(A0);
  if (history_write_position == history_read_position-1) {
    history_read_position = (history_read_position+1) % history_length;
  }

  history_time[history_write_position] = millis();
  history_value[history_write_position] = sensorValue;

  history_write_position = (history_write_position+1) % history_length;
  digitalWrite(LED, 0);
}


void setup() {
  pinMode ( LED, OUTPUT );
  digitalWrite ( LED, 0 );
  Serial.begin ( 115200 );
  
  WiFi.mode ( WIFI_STA );
  WiFi.begin ( ssid, password );
  Serial.println ( "" );

  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 100 );
    Serial.print ( "." );
  }

  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );

  if ( MDNS.begin ( myhostname ) ) {
    Serial.println ( "MDNS responder started" );
  }

  server.on ( "/", handleRoot );
  server.onNotFound ( handleNotFound );
  server.begin();
  Serial.println ( "HTTP server started" );

//  Blynk.config(blynkAuth);
  timer.setInterval(interval, sample);
}

void loop() {
  timer.run();
//  Blynk.run();
//  sample();
  server.handleClient();
}
