#include <Arduino.h>
#include "WiFi.h"
#include "esp-mqtt-wrapper.h"

// Update these with values suitable for your network.

const char* ssid = "........";
const char* password = "........";
const char* mqtt_server = "mqtts://test.mosquitto.org:8884";
const char* out_topic = "123456789/outTopic";
const char* in_topic = "123456789/inTopic";

extern const uint8_t root_ca_pem_start[] asm("_binary_src_certs_root_ca_crt_start");
extern const uint8_t root_ca_pem_end[] asm("_binary_src_certs_root_ca_crt_end");

extern const uint8_t certificate_pem_crt_start[] asm("_binary_src_certs_certificate_crt_start");
extern const uint8_t certificate_pem_crt_end[] asm("_binary_src_certs_certificate_crt_end");

extern const uint8_t private_pem_key_start[] asm("_binary_src_certs_private_key_start");
extern const uint8_t private_pem_key_end[] asm("_binary_src_certs_private_key_end");

esp_mqtt client;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE 50
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

void statusCallback(int32_t status) {
  if (status == MQTT_CONNECTED) {
    client.publish(out_topic, "hello world");
    client.subscribe(in_topic);
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
    digitalWrite(BUILTIN_LED, LOW);
    Serial.println("LED ON");
  } else {
    digitalWrite(BUILTIN_LED, HIGH);
    Serial.println("LED OFF");
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);  // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  Serial.println("Setup...");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.setStatusCallback(statusCallback);
  client.setRootCA((const char*)root_ca_pem_start);
  client.setClientCertificate((const char*)certificate_pem_crt_start);
  client.setClientKey((const char*)private_pem_key_start);
  client.connect("");
}

void loop() {
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    if (client.connected()) {
      ++value;
      snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish(out_topic, msg);
    }
  }
}