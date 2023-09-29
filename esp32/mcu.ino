#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Initialize BME280 sensor object
Adafruit_BME280 bme;

// Replace with your network credentials
const char* ssid = "SM24A";
const char* password = "SCSMWSST";

// Create an instance of the WebServer class on port 80
WebServer server(80);

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("HI!!");
  Wire.begin();
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (!SPIFFS.begin(true)) { // Initialize the SPI Flash File System
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }

  server.on("/", handleRoot);
  server.on("/api", handleDataRequest);
  server.begin();

  unsigned status;
  status = bme.begin(0x77);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    status = bme.begin(0x76);
  } else {
    Serial.println("Found");
  }
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String htmlContent;
  File file = SPIFFS.open("/index.html", "r"); // Open the index.html file

  if (file) {
    while (file.available()) {
      htmlContent += file.readStringUntil('\n');
    }
    file.close();

    // Read sensor values
    float temperature = bme.readTemperature();
    float pressure = bme.readPressure() / 100.0F;
    float humidity = bme.readHumidity();

    // Replace placeholders in the HTML with actual sensor values
    htmlContent.replace("{TEMPERATURE}", String(temperature));
    htmlContent.replace("{HUMIDITY}", String(humidity));
    htmlContent.replace("{PRESSURE}", String(pressure));

    server.send(200, "text/html", htmlContent);
  } else {
    server.send(404, "text/plain", "File Not Found");
  }
}

void handleDataRequest() {
  // Counter Anomaly
  float t = bme.readTemperature();
  if (t > 100) {
    ESP.restart();
  }
  
  String jsonPayload = "{";
  jsonPayload += "\"temperature\":";
  jsonPayload += t;
  jsonPayload += ",";
  jsonPayload += "\"pressure\":";
  jsonPayload += bme.readPressure() / 100.0F;
  jsonPayload += ",";
  jsonPayload += "\"humidity\":";
  jsonPayload += bme.readHumidity();
  jsonPayload += "}";
  server.send(200, "application/json", jsonPayload);
}
