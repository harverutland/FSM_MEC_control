// #include <WiFi.h>
// #include <ArduinoOTA.h>
// #include <Adafruit_MCP4728.h>
// #include <Wire.h>
// #include <HTTPClient.h>
// #include <ArduinoJson.h>

// // WiFi credentials
// const char *ssid = "TP-Link_70A6"; // Replace with your WiFi SSID
// const char *password = "59025306"; // Replace with your WiFi password

// // Flask server URL
// const char *serverUrl = "http://192.168.0.106:5000/data"; // Replace with your server's IP and port

// // Static IP address configuration
// IPAddress staticIP(192, 168, 0, 184); // Static IP address
// IPAddress gateway(192, 168, 0, 1);    // Gateway IP address
// IPAddress subnet(255, 255, 255, 0);   // Subnet mask

// // MCP4728 DAC
// Adafruit_MCP4728 mcp;

// unsigned long previousMillis = 0; // Will store last time data was sent
// const long interval = 10000;      // Interval at which to send data (10 seconds)
// unsigned long updateDACChannel(MCP4728_channel_t channel, int duration, unsigned long currentTime);

// void sendDataToServer();
// void updateDACChannels(const JsonDocument &doc); // Corrected prototype

// void setup()
// {
//   Serial.begin(115200);

//   WiFi.config(staticIP, gateway, subnet);
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(500);
//     Serial.println("Connecting to WiFi...");
//   }
//   Serial.print("Connected to WiFi, IP address: ");
//   Serial.println(WiFi.localIP());

//   ArduinoOTA.setHostname("MyDeviceName");
//   ArduinoOTA.begin();

//   if (!mcp.begin(0x64))
//   {
//     while (1)
//     {
//       delay(10);
//       Serial.println("Failed to find MCP4728 chip");
//     }
//   }

//   uint16_t dacValue = (1.8 / 3.3) * 4095;
//   mcp.setChannelValue(MCP4728_CHANNEL_A, dacValue);
//   mcp.setChannelValue(MCP4728_CHANNEL_B, dacValue);
//   mcp.setChannelValue(MCP4728_CHANNEL_C, dacValue);
//   mcp.setChannelValue(MCP4728_CHANNEL_D, dacValue);
//   Serial.println("DAC channels set to 1.8V");
// }

// void loop()
// {
//   ArduinoOTA.handle();

//   unsigned long currentMillis = millis();
//   if (currentMillis - previousMillis >= interval)
//   {
//     previousMillis = currentMillis;

//     if (WiFi.status() == WL_CONNECTED)
//     {
//       sendDataToServer();
//     }
//     else
//     {
//       Serial.println("WiFi not connected");
//     }
//   }
// }

// void sendDataToServer()
// {
//   HTTPClient http;
//   http.begin("http://192.168.0.106:5000/getdata"); // Flask server providing data
//   int httpCode = http.GET();

//   if (httpCode > 0)
//   {
//     String payload = http.getString();
//     Serial.println("Received data: " + payload);

//     DynamicJsonDocument doc(1024);
//     deserializeJson(doc, payload);

//     // Modify the 'dev' field
//     doc["dev"] = "ESP";

//     // Calculate the remaining high time for each channel
//     unsigned long currentTime = millis();
//     unsigned long remainingTimeA = updateDACChannel(MCP4728_CHANNEL_A, doc["p1"].as<int>(), currentTime);
//     unsigned long remainingTimeB = updateDACChannel(MCP4728_CHANNEL_B, doc["p2"].as<int>(), currentTime);
//     unsigned long remainingTimeC = updateDACChannel(MCP4728_CHANNEL_C, doc["p3"].as<int>(), currentTime);
//     unsigned long remainingTimeD = updateDACChannel(MCP4728_CHANNEL_D, doc["p4"].as<int>(), currentTime);

//     DynamicJsonDocument newDoc(1024);
//     newDoc["dev"] = doc["dev"];

//     // Conditionally add p1, p2, p3, p4 and their remaining times to the new JSON document
//     if (remainingTimeA > 0)
//     {
//       newDoc["p1"] = doc["p1"].as<int>();
//       newDoc["remainingTimeA"] = remainingTimeA;
//     }
//     if (remainingTimeB > 0)
//     {
//       newDoc["p2"] = doc["p2"].as<int>();
//       newDoc["remainingTimeB"] = remainingTimeB;
//     }
//     if (remainingTimeC > 0)
//     {
//       newDoc["p3"] = doc["p3"].as<int>();
//       newDoc["remainingTimeC"] = remainingTimeC;
//     }
//     if (remainingTimeD > 0)
//     {
//       newDoc["p4"] = doc["p4"].as<int>();
//       newDoc["remainingTimeD"] = remainingTimeD;
//     }

//     // Serialize the new modified JSON back to a string
//     String modifiedPayload;
//     serializeJson(newDoc, modifiedPayload);
//     Serial.println("Processed payload: " + modifiedPayload);

//     // Send modified payload back to the server
//     http.begin("http://192.168.0.106:5000/data"); // Flask server receiving data
//     http.addHeader("Content-Type", "application/x-www-form-urlencoded");
//     String httpResponseData = "response=" + modifiedPayload;
//     int httpResponseCode = http.POST(httpResponseData);

//     if (httpResponseCode > 0)
//     {
//       Serial.println("Response sent. Code: " + String(httpResponseCode));
//     }
//     else
//     {
//       Serial.println("Error sending response: " + String(httpResponseCode));
//     }
//   }
//   else
//   {
//     Serial.println("Error receiving data: " + String(httpCode));
//   }
//   http.end();
// }


// void updateDACChannels(const JsonDocument &doc)
// {
//   unsigned long currentTime = millis();
//   updateDACChannel(MCP4728_CHANNEL_A, doc["p1"].as<int>(), currentTime);
//   updateDACChannel(MCP4728_CHANNEL_B, doc["p2"].as<int>(), currentTime);
//   updateDACChannel(MCP4728_CHANNEL_C, doc["p3"].as<int>(), currentTime);
//   updateDACChannel(MCP4728_CHANNEL_D, doc["p4"].as<int>(), currentTime);
// }

// unsigned long updateDACChannel(MCP4728_channel_t channel, int duration, unsigned long currentTime)
// {
//   static unsigned long channelEndTime[4] = {0, 0, 0, 0};
//   unsigned long remainingTime = 0;

//   if (duration > 0)
//   {
//     uint16_t highValue = (1.8 / 3.3) * 4095;
//     mcp.setChannelValue(channel, highValue);
//     channelEndTime[channel] = currentTime + duration;
//   }
//   else if (currentTime >= channelEndTime[channel])
//   {
//     mcp.setChannelValue(channel, 0);
//   }

//   if (currentTime < channelEndTime[channel])
//   {
//     remainingTime = channelEndTime[channel] - currentTime;
//   }

//   return remainingTime;
// }


// #######################







#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Adafruit_MCP4728.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char *ssid = "TP-Link_70A6"; // Replace with your WiFi SSID
const char *password = "59025306"; // Replace with your WiFi password

// Flask server URL
const char *serverUrl = "http://192.168.0.106:5000/data"; // Replace with your server's IP and port

// Static IP address configuration
IPAddress staticIP(192, 168, 0, 184); // Static IP address
IPAddress gateway(192, 168, 0, 1);    // Gateway IP address
IPAddress subnet(255, 255, 255, 0);   // Subnet mask

// MCP4728 DAC
Adafruit_MCP4728 mcp;

unsigned long previousMillis = 0; // Will store last time data was sent
const long interval = 10000;      // Interval at which to send data (10 seconds)
unsigned long updateDACChannel(MCP4728_channel_t channel, int duration, unsigned long currentTime);

void sendDataToServer();
void updateDACChannels(const JsonDocument &doc); // Corrected prototype

void setup()
{
  Serial.begin(115200);

  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("Connected to WiFi, IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname("MyDeviceName");
  ArduinoOTA.begin();

  if (!mcp.begin(0x64))
  {
    while (1)
    {
      delay(10);
      Serial.println("Failed to find MCP4728 chip");
    }
  }

  uint16_t dacValue = (1.8 / 3.3) * 4095;
  mcp.setChannelValue(MCP4728_CHANNEL_A, dacValue);
  mcp.setChannelValue(MCP4728_CHANNEL_B, dacValue);
  mcp.setChannelValue(MCP4728_CHANNEL_C, dacValue);
  mcp.setChannelValue(MCP4728_CHANNEL_D, dacValue);
  Serial.println("DAC channels set to 1.8V");
}

void loop()
{
  ArduinoOTA.handle();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    if (WiFi.status() == WL_CONNECTED)
    {
      sendDataToServer();
    }
    else
    {
      Serial.println("WiFi not connected");
    }
  }
}

void sendDataToServer()
{
  HTTPClient http;
  http.begin("http://192.168.0.106:5000/getdata"); // Flask server providing data
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println("Received data: " + payload);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    // Modify the 'dev' field
    doc["dev"] = "ESP";

    // Calculate the remaining high time for each channel
    unsigned long currentTime = millis();
    unsigned long remainingTimeA = updateDACChannel(MCP4728_CHANNEL_A, doc["p1"].as<int>(), currentTime);
    unsigned long remainingTimeB = updateDACChannel(MCP4728_CHANNEL_B, doc["p2"].as<int>(), currentTime);
    unsigned long remainingTimeC = updateDACChannel(MCP4728_CHANNEL_C, doc["p3"].as<int>(), currentTime);
    unsigned long remainingTimeD = updateDACChannel(MCP4728_CHANNEL_D, doc["p4"].as<int>(), currentTime);

    DynamicJsonDocument newDoc(1024);
    newDoc["dev"] = doc["dev"];

    // Conditionally add p1, p2, p3, p4 and their remaining times to the new JSON document
    if (remainingTimeA > 0)
    {
      newDoc["p1"] = doc["p1"].as<int>();
      newDoc["remainingTimeA"] = remainingTimeA;
    }
    if (remainingTimeB > 0)
    {
      newDoc["p2"] = doc["p2"].as<int>();
      newDoc["remainingTimeB"] = remainingTimeB;
    }
    if (remainingTimeC > 0)
    {
      newDoc["p3"] = doc["p3"].as<int>();
      newDoc["remainingTimeC"] = remainingTimeC;
    }
    if (remainingTimeD > 0)
    {
      newDoc["p4"] = doc["p4"].as<int>();
      newDoc["remainingTimeD"] = remainingTimeD;
    }

    // Serialize the new modified JSON back to a string
    String modifiedPayload;
    serializeJson(newDoc, modifiedPayload);
    Serial.println("Processed payload: " + modifiedPayload);

    // Send modified payload back to the server
    http.begin("http://192.168.0.106:5000/data"); // Flask server receiving data
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpResponseData = "response=" + modifiedPayload;
    int httpResponseCode = http.POST(httpResponseData);

    if (httpResponseCode > 0)
    {
      Serial.println("Response sent. Code: " + String(httpResponseCode));
    }
    else
    {
      Serial.println("Error sending response: " + String(httpResponseCode));
    }
  }
  else
  {
    Serial.println("Error receiving data: " + String(httpCode));
  }
  http.end();
}


void updateDACChannels(const JsonDocument &doc)
{
  unsigned long currentTime = millis();
  updateDACChannel(MCP4728_CHANNEL_A, doc["p1"].as<int>(), currentTime);
  updateDACChannel(MCP4728_CHANNEL_B, doc["p2"].as<int>(), currentTime);
  updateDACChannel(MCP4728_CHANNEL_C, doc["p3"].as<int>(), currentTime);
  updateDACChannel(MCP4728_CHANNEL_D, doc["p4"].as<int>(), currentTime);
}

unsigned long updateDACChannel(MCP4728_channel_t channel, int duration, unsigned long currentTime) {
    static unsigned long channelEndTime[4] = {0, 0, 0, 0};
    static unsigned long channelStartTime[4] = {0, 0, 0, 0};
    unsigned long remainingTime = 0;

    if (duration > 0) {
        if (channelStartTime[channel] == 0) { // if the channel is being triggered for the first time
            channelStartTime[channel] = currentTime; // record the start time
            mcp.setChannelValue(channel, 0); // set DAC to 0V
        } else if (currentTime - channelStartTime[channel] < 240000) { // if it's less than 1 minute since the start
            mcp.setChannelValue(channel, 0); // keep DAC at 0V
        } else {
            uint16_t highValue = (1.8 / 3.3) * 4095;
            mcp.setChannelValue(channel, highValue); // set DAC to 1.8V
            channelEndTime[channel] = currentTime + duration;
        }
    } else if (currentTime >= channelEndTime[channel]) {
        mcp.setChannelValue(channel, 0); // set DAC to 0V
        channelStartTime[channel] = 0; // reset the start time
    }

    if (currentTime < channelEndTime[channel]) {
        remainingTime = channelEndTime[channel] - currentTime;
    }

    return remainingTime;
}





