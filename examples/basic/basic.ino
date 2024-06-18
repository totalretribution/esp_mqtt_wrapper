#include <Arduino.h>
#include "WiFi.h"
#include "esp-mqtt-wrapper.h"

// #define USE_STATUS_CALLBACK

// Update these with values suitable for your network.

const char* ssid = "........";
const char* password = "........";
const char* mqtt_server = "broker.mqtt-dashboard.com";

esp_mqtt client;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

#ifdef USE_STATUS_CALLBACK
void statusCallback(int32_t status) {
  if (status == MQTT_CONNECTED) {
    client.publish("outTopic", "hello world");
    client.subscribe("inTopic");
    return;
  }
  if (status == MQTT_DISCONNECTED) {
    return;
  }
  if (status == MQTT_CONNECTING) {
    return;
  }
  if (status == MQTT_CONNECT_FAILED) {
    return;
  }
}
#endif

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.print("strlen:");
  Serial.println(strlen(topic));

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    // digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    Serial.println("LED ON");
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    // digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    Serial.println("LED OFF");
  }
}

#ifndef USE_STATUS_CALLBACK
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
#endif

void setup() {
  // pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  Serial.println("Setup...");
  setup_wifi();
  // client.setServer(mqtt_server, 1883);
  client.setServer("mqtt://192.168.0.2", 1883);
  client.setCallback(callback);
#ifdef USE_STATUS_CALLBACK
  client.setStatusCallback(statusCallback);
  client.connect("");
#endif
}

void loop() {
#ifndef USE_STATUS_CALLBACK
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
#endif
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    if (client.connected()) {
      ++value;
      snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("outTopic", msg);
    }
  }
}