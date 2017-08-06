#include <ESP8266WiFi.h>
#include <BME280_MOD-1022.h>
#include <Wire.h>
#include "consts.h"

#define LED_PIN 12
#define I2C_SDA_PIN 13
#define I2C_SCL_PIN 14

WiFiServer server(80);

void setup_BME280() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  delay(10);

  BME280.readCompensationParams();

  BME280.writeStandbyTime(tsb_0p5ms);
  BME280.writeFilterCoefficient(fc_16);
  BME280.writeOversamplingTemperature(os1x);
  BME280.writeOversamplingHumidity(os1x);
  BME280.writeOversamplingPressure(os1x);

  BME280.writeMode(smNormal);
}

void setup_wifi() {
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(10);

  setup_BME280();
  
  setup_wifi();

  // prepare LED_PIN
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 0);
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  digitalWrite(LED_PIN, HIGH);
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  if ( req.indexOf("/bme280") == -1 ) {
    Serial.println("invalid request");
    client.stop();
    return;
  }
 
  double tempMostAccurate, humidityMostAccurate, pressureMostAccurate;
  BME280.readMeasurements(); 
  tempMostAccurate     = BME280.getTemperatureMostAccurate();
  humidityMostAccurate = BME280.getHumidityMostAccurate();
  pressureMostAccurate = BME280.getPressureMostAccurate();
  
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<h2>B4ME280</h2>\r\n";
  s += "temp : " + String(tempMostAccurate) + " [℃]<br>\r\n";
  s += "humidity : " + String(humidityMostAccurate) + " [%]<br>\r\n";
  s += "pressure : " + String(pressureMostAccurate) + " [hPa]<br>\r\n";
  s += "</html>\r\n";

  client.print(s);
  delay(10);

  digitalWrite(LED_PIN, LOW);
  Serial.println("Client disonnected");
}
