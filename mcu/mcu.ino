#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>

// Initialize BME280 sensor object
Adafruit_BME280 bme;

// Replace with your network credentials
const char* ssid = "{YOUR_SSID}";
const char* password = "{YOUR_PASSWORD}";

// Create an instance of the ESP8266WebServer class on port 80
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Wait for Wi-Fi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  SPIFFS.begin(); // Initialize the SPI Flash File System

  // Define request handlers
  server.on("/", handleRoot);
  server.on("/api", handleDataRequest);
  server.begin();

  // Initialize BME280 sensor
  unsigned status;
  status = bme.begin(0x77);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    while (1) delay(10);
  }
}

void loop() {
  server.handleClient(); // Handle client requests
}

// Handle root request and serve index.html
void handleRoot() {
  String htmlContent;
  File file = SPIFFS.open("/index.html", "r"); // Open the index.html file

  if (file) {
    while (file.available()) {
      htmlContent += file.readStringUntil('\n');
    }
    file.close();
    server.send(200, "text/html", htmlContent);
  } else {
    server.send(404, "text/plain", "File Not Found");
  }
}

// Handle data request and serve sensor readings as JSON
void handleDataRequest() {
  // Create a JSON payload with sensor data
  String jsonPayload = "{";
  jsonPayload += "\"temperature\":";
  jsonPayload += bme.readTemperature();
  jsonPayload += ",";
  jsonPayload += "\"pressure\":";
  jsonPayload += bme.readPressure() / 100.0F;
  jsonPayload += ",";
  jsonPayload += "\"humidity\":";
  jsonPayload += bme.readHumidity();
  jsonPayload += "}";

  // Send JSON response with sensor data
  server.send(200, "application/json", jsonPayload);
}
