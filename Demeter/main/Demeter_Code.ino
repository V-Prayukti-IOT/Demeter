#include <Wire.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Include necessary library for Serial communication
#include <Arduino.h>

// Defining Pins
#define DHTPIN 12
#define SOIL_MOISTURE_PIN A0  // Analog pin connected to the soil moisture sensor
#define LED_PIN 2             // Change this to the actual pin connected to the LED on ESP8266
#define LED2_PIN 14

// DHT parameters
#define DHTTYPE DHT11  // DHT 
DHT dht(DHTPIN, DHTTYPE);

// Soil Moisture Sensor
const int soilMoisturePin = SOIL_MOISTURE_PIN;
int soilMoistureValue = 0;

// Rolling average parameters for soil moisture
const int numReadings = 10;  // Number of readings to average
int soilMoistureReadings[numReadings];  // Array to store readings
int soilMoistureIndex = 0;  // Index to store current reading
int totalMoisture = 0;  // Variable to store the total sum of readings

// MQTT Credentials
const char* ssid = "KRISH 6673";           // Setting your AP SSID
const char* password = "krish12345";  // Setting your AP PSK
const char* mqttServer = "192.168.137.187";  // Remove the "mqtt://" prefix
const char* clientID = "ESP-8266";           // Client ID username+0001
const char* topicTemp = "Tempdata";         // Publish topic for temperature and humidity
const char* topicSoil = "SoilMoisture";     // Publish topic for soil moisture

// Setting up WiFi and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

// Non-blocking delay parameters
unsigned long previousMillis = 0;
const long interval = 1000;  // Interval in milliseconds

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(clientID)) {
      Serial.println("MQTT connected");
      client.subscribe("lights");
      Serial.println("Topic Subscribed");
      client.subscribe("lights2");
      Serial.println("Topic Subscribed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // wait 5sec and retry
    }
  }
}

// Subscribe callback
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String data = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    data += (char)payload[i];
  }
  Serial.println();
  Serial.print("Message size: ");
  Serial.println(length);
  Serial.println();
  Serial.println("-----------------------");
  Serial.println(data);

  if (String(topic) == "lights2") {
    if (data == "ON") {
      Serial.println("LED2");
      digitalWrite(LED2_PIN, HIGH);
    } else {
      digitalWrite(LED2_PIN, LOW);
    }
  }

  if (String(topic) == "lights") {
    if (data == "ON") {
      Serial.println("LED");
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize device.
  dht.begin();
  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED2_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize soil moisture readings array
  for (int i = 0; i < numReadings; i++) {
    soilMoistureReadings[i] = 0;
  }

  setup_wifi();
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Read temperature and humidity
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    // Read soil moisture value
    totalMoisture = totalMoisture - soilMoistureReadings[soilMoistureIndex];
    soilMoistureReadings[soilMoistureIndex] = analogRead(soilMoisturePin);
    totalMoisture = totalMoisture + soilMoistureReadings[soilMoistureIndex];
    soilMoistureIndex = (soilMoistureIndex + 1) % numReadings;

    soilMoistureValue = totalMoisture / numReadings;

    // Map soil moisture value to a percentage between 0% and 100%
    soilMoistureValue = map(soilMoistureValue, 0, 1023, 0, 100);

    if (isnan(temp) || isnan(hum)) {
      Serial.println(F("Error reading temperature or humidity!"));
    } else {
      Serial.print(F("Temperature: "));
      Serial.print(temp);
      Serial.println(F("°C"));
      Serial.print(F("Humidity: "));
      Serial.print(hum);
      Serial.println(F("%"));
      Serial.print("Soil Moisture Value: ");
      Serial.print(soilMoistureValue);
      Serial.println("%");
    }

    // Publish temperature and humidity data to MQTT
    String msgTemp = String(temp) + "," + String(hum) + ",";
    client.publish(topicTemp, msgTemp.c_str());

    // Publish soil moisture data to MQTT
    String msgSoil = String(soilMoistureValue);
    client.publish(topicSoil, msgSoil.c_str());

    delay(1);
  }
}
