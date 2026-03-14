#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

/* -------- WiFi -------- */
const char* ssid = "Wokwi-GUEST";
const char* password = "";

/* -------- MQTT -------- */
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

/* -------- Sensor Pins -------- */
#define DHTPIN 4
#define DHTTYPE DHT22
#define LDR_PIN 34
#define PIR_PIN 27
#define TRIG_PIN 5
#define ECHO_PIN 18

/* -------- Objects -------- */
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

/* -------- Timing -------- */
unsigned long lastPublish = 0;
unsigned long lastHeartbeat = 0;

const int publishInterval = 2000;     // sensor publish every 2 seconds
const int heartbeatInterval = 10000;  // system status every 10 seconds


/* -------- WiFi Connection -------- */
void setupWiFi() {

  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
}


/* -------- MQTT Reconnect -------- */
void reconnectMQTT() {

  while (!client.connected()) {

    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP32_Client")) {
      Serial.println("MQTT Connected!");
    } else {
      Serial.println("MQTT failed. Retrying...");
      delay(2000);
    }
  }
}


/* -------- Setup -------- */
void setup() {

  Serial.begin(115200);

  dht.begin();

  pinMode(LDR_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  setupWiFi();

  client.setServer(mqtt_server, mqtt_port);
}


/* -------- Read Ultrasonic -------- */
float readDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;

  return distance;
}


/* -------- Publish Sensor Data -------- */
void publishSensors() {

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  int lightLevel = analogRead(LDR_PIN);
  int motion = digitalRead(PIR_PIN);

  float distance = readDistance();

  Serial.println("------ SENSOR DATA ------");

  Serial.print("Temperature: ");
  Serial.println(temperature);

  Serial.print("Humidity: ");
  Serial.println(humidity);

  Serial.print("Light: ");
  Serial.println(lightLevel);

  Serial.print("Motion: ");
  Serial.println(motion);

  Serial.print("Distance: ");
  Serial.println(distance);

  /* JSON Payloads */

  String tempPayload =
      "{\"value\": " + String(temperature) + ", \"unit\": \"C\"}";
  client.publish("sensors/temperature", tempPayload.c_str());

  String humPayload =
      "{\"value\": " + String(humidity) + ", \"unit\": \"%\"}";
  client.publish("sensors/humidity", humPayload.c_str());

  String lightPayload =
      "{\"value\": " + String(lightLevel) + "}";
  client.publish("sensors/light", lightPayload.c_str());

  String motionPayload =
      "{\"detected\": " + String(motion) + "}";
  client.publish("sensors/motion", motionPayload.c_str());

  String distPayload =
      "{\"value\": " + String(distance) + ", \"unit\": \"cm\"}";
  client.publish("sensors/distance", distPayload.c_str());
}


/* -------- Heartbeat -------- */
void publishStatus() {

  String statusPayload = "{\"status\":\"online\"}";

  client.publish("system/status", statusPayload.c_str());

  Serial.println("System heartbeat sent.");
}


/* -------- Loop -------- */
void loop() {

  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();

  unsigned long now = millis();

  /* Publish sensors */
  if (now - lastPublish > publishInterval) {

    publishSensors();
    lastPublish = now;
  }

  /* Publish system heartbeat */
  if (now - lastHeartbeat > heartbeatInterval) {

    publishStatus();
    lastHeartbeat = now;
  }
}
