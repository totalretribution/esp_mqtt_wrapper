#include "esp-mqtt-wrapper.h"
#include "Arduino.h"

// public:

esp_mqtt::esp_mqtt() {
  this->_state = MQTT_DISCONNECTED;
  setCallback(NULL);
  setStatusCallback(NULL);
  this->bufferSize = 0;
  setBufferSize(MQTT_MAX_PACKET_SIZE);
  setKeepAlive(MQTT_KEEPALIVE);
  setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

esp_mqtt::esp_mqtt(IPAddress ip, uint16_t port) {
  this->_state = MQTT_DISCONNECTED;
  setServer(ip, port);
  setCallback(NULL);
  setStatusCallback(NULL);
  this->bufferSize = 0;
  setBufferSize(MQTT_MAX_PACKET_SIZE);
  setKeepAlive(MQTT_KEEPALIVE);
  setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

esp_mqtt::esp_mqtt(IPAddress ip, uint16_t port, MQTT_CALLBACK_SIGNATURE) {
  this->_state = MQTT_DISCONNECTED;
  setServer(ip, port);
  setCallback(message_cb);
  setStatusCallback(NULL);
  this->bufferSize = 0;
  setBufferSize(MQTT_MAX_PACKET_SIZE);
  setKeepAlive(MQTT_KEEPALIVE);
  setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

esp_mqtt::esp_mqtt(const char* domain, uint16_t port) {
  this->_state = MQTT_DISCONNECTED;
  setServer(domain, port);
  setCallback(NULL);
  setStatusCallback(NULL);
  this->bufferSize = 0;
  setBufferSize(MQTT_MAX_PACKET_SIZE);
  setKeepAlive(MQTT_KEEPALIVE);
  setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

esp_mqtt::esp_mqtt(const char* domain, uint16_t port, MQTT_CALLBACK_SIGNATURE) {
  this->_state = MQTT_DISCONNECTED;
  setServer(domain, port);
  setCallback(message_cb);
  setStatusCallback(NULL);
  this->bufferSize = 0;
  setBufferSize(MQTT_MAX_PACKET_SIZE);
  setKeepAlive(MQTT_KEEPALIVE);
  setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

esp_mqtt::~esp_mqtt() {}

esp_mqtt& esp_mqtt::setServer(IPAddress ip, uint16_t port) {
  this->ip = ip;
  this->port = port;
  this->domain = NULL;
  return *this;
}

esp_mqtt& esp_mqtt::setServer(uint8_t* ip, uint16_t port) {
  IPAddress addr(ip[0], ip[1], ip[2], ip[3]);
  return setServer(addr, port);
}

esp_mqtt& esp_mqtt::setServer(const char* domain, uint16_t port) {
  this->domain = domain;
  this->port = port;
  return *this;
}

esp_mqtt& esp_mqtt::setCallback(MQTT_CALLBACK_SIGNATURE) {
  this->message_cb = message_cb;
  return *this;
}

esp_mqtt& esp_mqtt::setStatusCallback(MQTT_CALLBACK_STATUS_SIGNATURE) {
  this->status_cb = status_cb;
  return *this;
}

esp_mqtt& esp_mqtt::setKeepAlive(uint16_t keepAlive) {
  this->keepAlive = keepAlive;
  return *this;
}

esp_mqtt& esp_mqtt::setSocketTimeout(uint16_t timeout) {
  this->socketTimeout = timeout;
  return *this;
}

boolean esp_mqtt::setBufferSize(uint16_t size) {
  if (size == 0) {
    return false;

    this->bufferSize = size;
    return true;
  }
}

uint16_t esp_mqtt::getBufferSize() {
  return this->bufferSize;
}

boolean esp_mqtt::connect(const char* id) {
  return connect(id, NULL, NULL, 0, 0, 0, 0, 1);
}

boolean esp_mqtt::connect(const char* id, const char* user, const char* pass) {
  return connect(id, user, pass, 0, 0, 0, 0, 1);
}

boolean esp_mqtt::connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage) {
  return connect(id, NULL, NULL, willTopic, willQos, willRetain, willMessage, 1);
}

boolean esp_mqtt::connect(const char* id,
                          const char* user,
                          const char* pass,
                          const char* willTopic,
                          uint8_t willQos,
                          boolean willRetain,
                          const char* willMessage) {
  return connect(id, user, pass, willTopic, willQos, willRetain, willMessage, 1);
}

boolean esp_mqtt::connect(const char* id,
                          const char* user,
                          const char* pass,
                          const char* willTopic,
                          uint8_t willQos,
                          boolean willRetain,
                          const char* willMessage,
                          boolean cleanSession) {
  if (connected())
    return false;

  esp_mqtt_client_config_t mqtt_cfg;
  // TODO: Build mqtt_cfg
  if (strcmp(id, "") == 0) {
    // TODO: Generate id
    mqtt_cfg.client_id = id;
  } else {
    mqtt_cfg.client_id = id;
  }

  if (_mqtt_handle)
    esp_mqtt_client_destroy(_mqtt_handle);

  if (!(_mqtt_handle = esp_mqtt_client_init(&mqtt_cfg)))
    return false;

  esp_mqtt_client_register_event(_mqtt_handle, MQTT_EVENT_ANY, esp_mqtt::s_handle_mqtt_event, this);
  esp_mqtt_client_start(_mqtt_handle);
}

void esp_mqtt::disconnect() {
    //TODO: add implementation
}

boolean esp_mqtt::publish(const char* topic, const char* payload) {
  return publish(topic, (const uint8_t*)payload, 0, false);
}

boolean esp_mqtt::publish(const char* topic, const char* payload, boolean retained) {
  return publish(topic, (const uint8_t*)payload, 0, retained);
}

boolean esp_mqtt::publish(const char* topic, const uint8_t* payload, unsigned int plength) {
  return publish(topic, payload, plength, false);
}

boolean esp_mqtt::publish(const char* topic, const uint8_t* payload, unsigned int plength, boolean retained) {
  int msg_id = esp_mqtt_client_publish(_mqtt_handle, topic, (const char*)payload, plength, 1, retained);
  if (msg_id != -1) {
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
  } else {
    ESP_LOGI(TAG, "sent publish unsuccessful, msg_id=%d", msg_id);
  }
  return msg_id != ESP_FAIL;
}

boolean esp_mqtt::publish_P(const char* topic, const char* payload, boolean retained) {
  return publish(topic, (const uint8_t*)payload, 0, retained);
}

boolean esp_mqtt::publish_P(const char* topic, const uint8_t* payload, unsigned int plength, boolean retained) {
  return publish(topic, payload, plength, retained);
}

boolean esp_mqtt::subscribe(const char* topic) {
  return subscribe(topic, 0);
}
boolean esp_mqtt::subscribe(const char* topic, uint8_t qos) {
  int msg_id = esp_mqtt_client_subscribe(_mqtt_handle, topic, qos);
  if (msg_id != -1) {
    ESP_LOGI(TAG, "sent subscribe successful, topic=%s", topic);
  } else {
    ESP_LOGI(TAG, "sent subscribe unsuccessful, topic=%s", topic);
  }
  return msg_id != ESP_FAIL;
}

boolean esp_mqtt::unsubscribe(const char* topic) {
  int msg_id = esp_mqtt_client_unsubscribe(_mqtt_handle, topic);
  if (msg_id != -1) {
    ESP_LOGI(TAG, "sent unsubscribe successful, topic=%s", topic);
  } else {
    ESP_LOGI(TAG, "sent unsubscribe unsuccessful, topic=%s", topic);
  }
  return msg_id != ESP_FAIL;
}

boolean esp_mqtt::loop() {
    return connected();
}

boolean esp_mqtt::connected() {
  return this->_state == MQTT_CONNECTED;
}

int esp_mqtt::state() {
  return this->_state;
}

// private:

void esp_mqtt::log_error_if_nonzero(const char* message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

/**
 * Static Event handler for handling MQTT event handler in a class.
 *
 * @param event_handler_arg The user data registered to the event.
 * @param event_base Event base for the handler (always MQTT Base in this example).
 * @param event_id The ID for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
void esp_mqtt::s_handle_mqtt_event(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  static_cast<esp_mqtt*>(event_handler_arg)->mqtt_event_handler(event_base, event_id, event_data);
}

/**
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
void esp_mqtt::mqtt_event_handler(esp_event_base_t event_base, int32_t event_id, void* event_data) {
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_DATA:
      ESP_LOGI(TAG, "MQTT_EVENT_DATA");
      message_cb(event->topic, (uint8_t*)event->data, event->data_len);
      break;
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
      this->_state = MQTT_CONNECTED;
      status_cb(event_id);
      break;
    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
      this->_state = MQTT_DISCONNECTED;
      status_cb(event_id);
      break;
    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_UNSUBSCRIBED:
      ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_ERROR:
      ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
      if (event->error_handle->connect_return_code != 0) {
        if (event->error_handle->connect_return_code == esp_mqtt_connect_return_code_t::MQTT_CONNECTION_REFUSE_PROTOCOL)
          _state = MQTT_CONNECT_BAD_PROTOCOL;
        else if (event->error_handle->connect_return_code == esp_mqtt_connect_return_code_t::MQTT_CONNECTION_REFUSE_ID_REJECTED)
          _state = MQTT_CONNECT_BAD_CLIENT_ID;
        else if (event->error_handle->connect_return_code ==
                 esp_mqtt_connect_return_code_t::MQTT_CONNECTION_REFUSE_SERVER_UNAVAILABLE)
          _state = MQTT_CONNECT_UNAVAILABLE;
        else if (event->error_handle->connect_return_code == esp_mqtt_connect_return_code_t::MQTT_CONNECTION_REFUSE_BAD_USERNAME)
          _state = MQTT_CONNECT_BAD_CREDENTIALS;
        else if (event->error_handle->connect_return_code ==
                 esp_mqtt_connect_return_code_t::MQTT_CONNECTION_REFUSE_NOT_AUTHORIZED)
          _state = MQTT_CONNECT_UNAUTHORIZED;
        ESP_LOGI(TAG, "Connection Error String (%s)", strerror(event->error_handle->connect_return_code));
      } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
        log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
        log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
        log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
        ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
      } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
        ESP_LOGI(TAG, "Connection refused error: %d", event->error_handle->error_code);
        this->_state = MQTT_CONNECT_FAILED;
      }
      status_cb(event_id);
      break;
    default:
      ESP_LOGI(TAG, "Other event id:%d", event->event_id);
      break;
  }
}