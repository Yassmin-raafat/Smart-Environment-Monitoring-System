#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <ESP32Servo.h>

/* WiFi */
const char* ssid = "Wokwi-GUEST";
const char* password = "";

/* MQTT */
const char* mqtt_server = "test.mosquitto.org";

/* Sensors */
#define DHTPIN 4
#define DHTTYPE DHT22
#define LDR_PIN 34
#define PIR_PIN 27
#define TRIG_PIN 5
#define ECHO_PIN 18

/* Actuators */
#define LED_PIN 19
#define BUZZER_PIN 23
#define RELAY_PIN 12
#define SERVO_PIN 13

DHT dht(DHTPIN, DHTTYPE);
Servo servo;

WiFiClient espClient;
PubSubClient client(espClient);

/* Thresholds */
float tempThreshold = 20;
int lightThreshold = 500;
float distanceThreshold = 20;

/* WiFi setup */
void setupWiFi() {

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
}

/* MQTT reconnect */
void reconnect() {

  while (!client.connected()) {

    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP32Client")) {

      Serial.println("MQTT Connected");

      client.subscribe("actuators/led");
      client.subscribe("actuators/buzzer");
      client.subscribe("actuators/relay");
      client.subscribe("actuators/servo");

    } else {

      delay(2000);
    }
  }
}

/* MQTT callback */
void callback(char* topic, byte* payload, unsigned int length) {

  String message;

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == "actuators/led") {

    if (message == "on")
      digitalWrite(LED_PIN, HIGH);
    else
      digitalWrite(LED_PIN, LOW);
  }

  if (String(topic) == "actuators/buzzer") {

    if (message == "on")
      digitalWrite(BUZZER_PIN, HIGH);
    else
      digitalWrite(BUZZER_PIN, LOW);
  }

  if (String(topic) == "actuators/relay") {

    if (message == "on")
      digitalWrite(RELAY_PIN, HIGH);
    else
      digitalWrite(RELAY_PIN, LOW);
  }

  if (String(topic) == "actuators/servo") {

    int angle = message.toInt();
    servo.write(angle);
  }
}

/* Ultrasonic distance */
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

void setup() {

  Serial.begin(115200);

  dht.begin();

  pinMode(LDR_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  servo.attach(SERVO_PIN);

  setupWiFi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  /* Read Sensors */

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int lightLevel = analogRead(LDR_PIN);
  int motion = digitalRead(PIR_PIN);
  float distance = readDistance();

  Serial.println("------ SENSOR DATA ------");

  Serial.print("Temp: ");
  Serial.println(temperature);

  Serial.print("Humidity: ");
  Serial.println(humidity);

  Serial.print("Light: ");
  Serial.println(lightLevel);

  Serial.print("Motion: ");
  Serial.println(motion);

  Serial.print("Distance: ");
  Serial.println(distance);

  /* Automatic Control */

  if (temperature > tempThreshold)
    digitalWrite(LED_PIN, HIGH);
  else
    digitalWrite(LED_PIN, LOW);

  if (motion == 1)
    digitalWrite(BUZZER_PIN, HIGH);
  else
    digitalWrite(BUZZER_PIN, LOW);

  if (lightLevel < lightThreshold)
    digitalWrite(RELAY_PIN, HIGH);
  else
    digitalWrite(RELAY_PIN, LOW);

  if (distance < distanceThreshold)
    servo.write(90);
  else
    servo.write(0);

  /* MQTT Publish */

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

  client.publish("system/status", "{\"status\":\"online\"}");

  delay(2000);
}