#include <SimpleTimer.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <BlynkSimpleEsp8266.h>

#include "DHT.h"

#include "auth.h"

// Initialize DHT sensor.
DHT dht(D4, DHT22);

ESP8266WebServer server ( 80 );

const int LED = 13;

SimpleTimer timer;

const long int readDHTPeriod = 30 * 1000;
const long int readDHTRetryPeriod = 2 * 1000;
bool readDHTNeeded = true;

#define INDOOR

#ifdef INDOOR
const char *myhostname = "indoorsensor";
const int virtualPinOffset = 0;
#else
const char *myhostname = "outdoorsensor";
const int virtualPinOffset = 10;
#endif
WidgetLED uptodateLED(3+virtualPinOffset);
int uptodateLEDTimerID = -1;

float currentTemp, currentHum;

void handleRoot() {
  Serial.println ( "Root request" );
  digitalWrite(LED, 1);
  String s = "hello from esp8266!\nTemp";
  s += currentTemp;
  s += "\nHumidity";
  s += currentHum;
  server.send(200, "text/plain", s);
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

void uptodateLEDOff() {
  timer.deleteTimer(uptodateLEDTimerID);
  uptodateLED.off();
}
void uptodateLEDOn() {
  // Turn on the LED until one second before the next read.
  // So if the LED is on the data is "fresh".
  timer.deleteTimer(uptodateLEDTimerID);
  uptodateLED.on(); 
  uptodateLEDTimerID = timer.setTimeout(readDHTPeriod - 5000, []() {
    uptodateLED.off();
  });
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

  Blynk.config(blynkAuth);

  // Force a read every so often
  timer.setInterval(readDHTPeriod, []() {
    readDHTNeeded = true;
  });

  // Try to read at a tighter interval until we get a good result.
  timer.setInterval(readDHTRetryPeriod, []() {
    if (!readDHTNeeded) return;
    
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t)) {
      uptodateLEDOff();
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    currentTemp = t;
    Blynk.virtualWrite(0 + virtualPinOffset, currentTemp); 
    
    if (isnan(h)) { 
      uptodateLEDOff();
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    currentHum = h;

    if(currentHum < 2) {
      // For broken sensors send obviously illegal data
      currentHum = -666; 
    }
    
    Blynk.virtualWrite(1 + virtualPinOffset, currentHum); 

    if(currentHum > 0) {
      Blynk.virtualWrite(2 + virtualPinOffset, dht.computeHeatIndex(currentTemp, currentHum, false));
    } else {
      Blynk.virtualWrite(2 + virtualPinOffset, -666.0f);      
    }

    uptodateLEDOn();

    readDHTNeeded = false;
  });
}

void loop() {
  timer.run();
  Blynk.run();
  server.handleClient();
}
