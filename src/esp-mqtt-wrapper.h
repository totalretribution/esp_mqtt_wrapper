#ifndef ESP_MQTT_WRAPPER_H
#define ESP_MQTT_WRAPPER_H

#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "esp_log.h"
#include "mqtt_client.h"

#define MQTT_VERSION_3_1 3
#define MQTT_VERSION_3_1_1 4
#define MQTT_VERSION_5 5

// MQTT_VERSION : Pick the version
// #define MQTT_VERSION MQTT_VERSION_3_1
#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1
#endif

// MQTT_MAX_PACKET_SIZE : Maximum packet size. Override with setBufferSize().
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 256
#endif

// MQTT_KEEPALIVE : keepAlive interval in Seconds. Override with setKeepAlive()
#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 15
#endif

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds. Override with setSocketTimeout()
#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 15
#endif

#define MQTT_CONNECTION_TIMEOUT -4
#define MQTT_CONNECTION_LOST -3
#define MQTT_CONNECT_FAILED -2
#define MQTT_DISCONNECTED -1
#define MQTT_CONNECTED 0
#define MQTT_CONNECT_BAD_PROTOCOL 1
#define MQTT_CONNECT_BAD_CLIENT_ID 2
#define MQTT_CONNECT_UNAVAILABLE 3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED 5
#define MQTT_CONNECTING 6

#define MQTT_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, unsigned int) > message_cb
#define MQTT_CALLBACK_STATUS_SIGNATURE std::function<void(int32_t) > status_cb

class esp_mqtt {
 public:
  esp_mqtt();
  esp_mqtt(IPAddress ip, uint16_t port);
  esp_mqtt(IPAddress ip, uint16_t port, MQTT_CALLBACK_SIGNATURE);
  esp_mqtt(const char* domain, uint16_t port);
  esp_mqtt(const char* domain, uint16_t port, MQTT_CALLBACK_SIGNATURE);

  ~esp_mqtt();

  esp_mqtt& setServer(IPAddress ip, uint16_t port);
  esp_mqtt& setServer(uint8_t* ip, uint16_t port);
  esp_mqtt& setServer(const char* domain);
  esp_mqtt& setServer(const char* domain, uint16_t port);
  esp_mqtt& setCallback(MQTT_CALLBACK_SIGNATURE);
  esp_mqtt& setStatusCallback(MQTT_CALLBACK_STATUS_SIGNATURE);
  esp_mqtt& setKeepAlive(uint16_t keepAlive);
  esp_mqtt& setSocketTimeout(uint16_t timeout);
  
  esp_mqtt& setRootCA(const char * ca);
  esp_mqtt& setClientCertificate(const char * cert);
  esp_mqtt& setClientKey(const char * key);

  boolean setBufferSize(uint16_t size);
  uint16_t getBufferSize();

  boolean connect(const char* id);
  boolean connect(const char* id, const char* user, const char* pass);
  boolean connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
  boolean connect(const char* id,
                  const char* user,
                  const char* pass,
                  const char* willTopic,
                  uint8_t willQos,
                  boolean willRetain,
                  const char* willMessage);
  boolean connect(const char* id,
                  const char* user,
                  const char* pass,
                  const char* willTopic,
                  uint8_t willQos,
                  boolean willRetain,
                  const char* willMessage,
                  boolean cleanSession);
  void disconnect();
  boolean publish(const char* topic, const char* payload, boolean wait = true);
  boolean publish(const char* topic, const char* payload, boolean retained, boolean wait = true);
  boolean publish(const char* topic, const uint8_t* payload, unsigned int plength, boolean wait = true);
  boolean publish(const char* topic, const uint8_t* payload, unsigned int plength, boolean retained, boolean wait = true);
  boolean publish_P(const char* topic, const char* payload, boolean retained, boolean wait = true);
  boolean publish_P(const char* topic, const uint8_t* payload, unsigned int plength, boolean retained, boolean wait = true);

  boolean subscribe(const char* topic);
  boolean subscribe(const char* topic, uint8_t qos);
  boolean unsubscribe(const char* topic);
  boolean loop();
  boolean connected();
  int state();

 private:
  const char* TAG = "esp-mqtt-wrapper";
  esp_mqtt_client_handle_t _mqtt_handle = NULL;
  IPAddress ip;
  char ip_str[17];
  const char* domain;
  uint32_t port;
  int8_t _state;
  uint16_t keepAlive;
  uint16_t socketTimeout;
  MQTT_CALLBACK_SIGNATURE;
  MQTT_CALLBACK_STATUS_SIGNATURE;
  uint16_t bufferSize;
  bool connect_called = false;
  const char* root_ca = NULL;
  const char* client_cert = NULL;
  const char* client_key = NULL;

  void log_error_if_nonzero(const char* message, int error_code);
  static void s_handle_mqtt_event(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
  void mqtt_event_handler(esp_event_base_t event_base, int32_t event_id, void* event_data);
  virtual size_t write(uint8_t);
};

#endif