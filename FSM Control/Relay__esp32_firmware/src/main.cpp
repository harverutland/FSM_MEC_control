#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "SparkFun_Qwiic_Relay.h"

// Constants and global variables
#define RELAY_ADDR1 0x18
#define RELAY_ADDR2 0x19
Qwiic_Relay relay1(RELAY_ADDR1); 
Qwiic_Relay relay2(RELAY_ADDR2); 
const char *ssid = "****"; // Replace with your WiFi SSID
const char *password = "****"; // Replace with your WiFi password
const char *serverUrl = "http://****/getdata"; // Replace with your server's IP and port
IPAddress staticIP(**, **, **, **); 
IPAddress gateway(**, **, **, **);    
IPAddress subnet(**, **, **, **);   
unsigned long previousMillis = 0;
const long interval = 10000;
long delayTime = 600000; // Delay time in milliseconds

// New variables for non-blocking delay
bool relay1On = false;
bool relay2On = false;

bool working1 = true;
bool working2 = true;
unsigned long relay1StartTime = 0;
unsigned long relay2StartTime = 0;

// Function prototypes
void sendDataToServer();
void checkAndToggleRelays();

void setup() {
  Wire.begin(); 
  Serial.begin(115200);
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("Connected to WiFi, IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname("MyDeviceName");
  ArduinoOTA.begin();

  // if (!relay1.begin()){  
  //   working = false;}

  if (!relay1.begin()){  
    working1 = false;}


  if (!relay2.begin()){  
    working2 = false;}
}




void loop() {
  ArduinoOTA.handle();
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (WiFi.status() == WL_CONNECTED) {
      sendDataToServer();
    } else {
      Serial.println("WiFi not connected");
    }
  }
  
  checkAndToggleRelays();
}

void checkAndToggleRelays() {
  unsigned long currentMillis = millis();
  if (relay1On && currentMillis - relay1StartTime >= delayTime) {
    relay1.turnRelayOff();
    relay1On = false;
    Serial.println("Relay 1 turned off.");
  }

  if (relay2On && currentMillis - relay2StartTime >= delayTime) {
    relay2.turnRelayOff();
    relay2On = false;
    Serial.println("Relay 2 turned off.");
  }
}

void sendDataToServer() {
  HTTPClient http;
  http.begin(String(serverUrl)); // GET request to the serverUrl
  int httpCode = http.GET();

  if (httpCode > 0) {
    String originalPayload = http.getString();
    Serial.println("Received payload from GET request: ");
    Serial.println(originalPayload);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, originalPayload);

    int p1 = doc["p1"].as<int>();
    int p2 = doc["p2"].as<int>();
    int p3 = doc["p3"].as<int>();
    int p4 = doc["p4"].as<int>();

    if ((p1 != 0 || p2 != 0) && !relay1On) {
      relay1.turnRelayOn();
      relay1StartTime = millis();
      relay1On = true;
    }

    if ((p3 != 0 || p4 != 0) && !relay2On) {
      relay2.turnRelayOn();
      relay2StartTime = millis();
      relay2On = true;
    }



    doc["dev"] = "ESP_relay";

    // Create a new JSON document for the modified payload
    DynamicJsonDocument newDoc(1024);
    newDoc["dev"] = doc["dev"];

    // Add status for each relay to the JSON document
    // The status is determined by the relay1On and relay2On boolean variables
    newDoc["relay1Status"] = relay1On ? "on" : "off";
    newDoc["relay2Status"] = relay2On ? "on" : "off";
    newDoc["relay1working"] = working1;
    newDoc["relay2working"] = working2;

    // Serialize JSON to create the modified payload
    String modifiedPayload;
    serializeJson(newDoc, modifiedPayload);

    // Print the modified payload for debugging
    Serial.println("Modified payload with relay status: ");
    Serial.println(modifiedPayload);


    // Send the modified payload back to the server
    http.begin("http://****/data"); // POST request to a different endpoint
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    Serial.println("Sending modified payload with POST request: ");
    Serial.println(modifiedPayload);  // Print POST data for debugging

    int httpResponseCode = http.POST(modifiedPayload);
    if (httpResponseCode > 0) {
      Serial.println("Response sent. Code: " + String(httpResponseCode));
    } else {
      Serial.println("Error sending response: " + String(httpResponseCode));
    }
  } else {
    Serial.println("Error receiving data: " + String(httpCode));
  }
  http.end();
}
