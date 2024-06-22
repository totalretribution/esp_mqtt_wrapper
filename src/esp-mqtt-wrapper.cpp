#include "esp-mqtt-wrapper.h"
#include "Arduino.h"

// public:

esp_mqtt::esp_mqtt() {
  this->_state = MQTT_DISCONNECTED;
  setCallback(NULL);
  setStatusCallback(NULL);
  setBufferSize(MQTT_MAX_PACKET_SIZE);
  setKeepAlive(MQTT_KEEPALIVE);
  setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

esp_mqtt::esp_mqtt(IPAddress ip, uint16_t port) {
  this->_state = MQTT_DISCONNECTED;
  setServer(ip, port);
  setCallback(NULL);
  setStatusCallback(NULL);
  setBufferSize(MQTT_MAX_PACKET_SIZE);
  setKeepAlive(MQTT_KEEPALIVE);
  setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

esp_mqtt::esp_mqtt(IPAddress ip, uint16_t port, MQTT_CALLBACK_SIGNATURE) {
  this->_state = MQTT_DISCONNECTED;
  setServer(ip, port);
  setCallback(message_cb);
  setStatusCallback(NULL);
  setBufferSize(MQTT_MAX_PACKET_SIZE);
  setKeepAlive(MQTT_KEEPALIVE);
  setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

esp_mqtt::esp_mqtt(const char* domain, uint16_t port) {
  this->_state = MQTT_DISCONNECTED;
  setServer(domain, port);
  setCallback(NULL);
  setStatusCallback(NULL);
  setBufferSize(MQTT_MAX_PACKET_SIZE);
  setKeepAlive(MQTT_KEEPALIVE);
  setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

esp_mqtt::esp_mqtt(const char* domain, uint16_t port, MQTT_CALLBACK_SIGNATURE) {
  this->_state = MQTT_DISCONNECTED;
  setServer(domain, port);
  setCallback(message_cb);
  setStatusCallback(NULL);
  setBufferSize(MQTT_MAX_PACKET_SIZE);
  setKeepAlive(MQTT_KEEPALIVE);
  setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

esp_mqtt::~esp_mqtt() {}

esp_mqtt& esp_mqtt::setServer(IPAddress ip, uint16_t port) {
  this->ip = ip;
  strcpy(this->ip_str, this->ip.toString().c_str());
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
  if (size == 0)
    return false;

  this->bufferSize = size;
  return true;
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
  if (this->_state == MQTT_CONNECTED) {
    connect_called = true;
    return true;
  }

  if (_mqtt_handle)
    return false;

  ESP_LOGI(TAG, "Attempting to connect to MQTT client");

  esp_mqtt_client_config_t mqtt_cfg;
  memset(&mqtt_cfg, 0, sizeof(esp_mqtt_client_config_t));

#if defined(ESP_IDF_VERSION) && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 0)
  if (strlen(id) > 0) {
    mqtt_cfg.credentials.client_id = id;
  }

  if (user != NULL) {
    mqtt_cfg.credentials.username = user;
  }

  if (pass != NULL) {
    mqtt_cfg.credentials.authentication.password = pass;
  }

  mqtt_cfg.session.last_will.topic = willTopic;
  mqtt_cfg.session.last_will.msg = willMessage;
  mqtt_cfg.session.last_will.qos = willQos;
  mqtt_cfg.session.last_will.retain = willRetain;

  if (domain != NULL) {
    if (strstr(domain, "://") != NULL) {
      mqtt_cfg.broker.address.uri = domain;
    } else {
      mqtt_cfg.broker.address.hostname = domain;
      mqtt_cfg.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
      mqtt_cfg.broker.address.port = this->port;
    }
  } else {
    mqtt_cfg.broker.address.hostname = this->ip_str;
    mqtt_cfg.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
    mqtt_cfg.broker.address.port = this->port;
  }

  mqtt_cfg.session.keepalive = this->keepAlive;
  mqtt_cfg.network.timeout_ms = this->socketTimeout * 1000;  // Needs to be in ms.
  mqtt_cfg.buffer.size = this->bufferSize;
#else
  if (strlen(id) > 0) {
    mqtt_cfg.client_id = id;
  }

  if (user != NULL) {
    mqtt_cfg.username = user;
  }

  if (pass != NULL) {
    mqtt_cfg.password = pass;
  }

  mqtt_cfg.lwt_topic = willTopic;
  mqtt_cfg.lwt_msg = willMessage;
  mqtt_cfg.lwt_qos = willQos;
  mqtt_cfg.lwt_retain = willRetain;

  if (domain != NULL) {
    if (strstr(domain, "://") != NULL) {
      mqtt_cfg.uri = domain;
    } else {
      mqtt_cfg.host = domain;
      mqtt_cfg.transport = MQTT_TRANSPORT_OVER_TCP;
      mqtt_cfg.port = this->port;
    }
  } else {
    mqtt_cfg.host = = this->ip_str;
    mqtt_cfg.transport = MQTT_TRANSPORT_OVER_TCP;
    mqtt_cfg.port = this->port;
  }

  mqtt_cfg.keepalive = this->keepAlive;
  mqtt_cfg.network_timeout_ms = this->socketTimeout * 1000;  // Needs to be in ms.
  mqtt_cfg.buffer_size = this->bufferSize;
#endif

  if (_mqtt_handle)
    esp_mqtt_client_destroy(_mqtt_handle);

  if (!(_mqtt_handle = esp_mqtt_client_init(&mqtt_cfg))) {
    esp_mqtt_client_destroy(_mqtt_handle);
    return false;
  }

  esp_mqtt_client_register_event(_mqtt_handle, MQTT_EVENT_ANY, esp_mqtt::s_handle_mqtt_event, this);
  esp_mqtt_client_start(_mqtt_handle);
  _state = MQTT_CONNECTING;
  if (status_cb != nullptr)
    status_cb(this->_state);

  return false;
}

void esp_mqtt::disconnect() {
  ESP_LOGI(TAG, "Disconnecting MQTT client");
  if (_mqtt_handle)
    esp_mqtt_client_destroy(_mqtt_handle);
  this->_state = MQTT_DISCONNECTED;
  _mqtt_handle = NULL;
}

boolean esp_mqtt::publish(const char* topic, const char* payload, boolean wait) {
  return publish(topic, (const uint8_t*)payload, 0, false, wait);
}

boolean esp_mqtt::publish(const char* topic, const char* payload, boolean retained, boolean wait) {
  return publish(topic, (const uint8_t*)payload, 0, retained, wait);
}

boolean esp_mqtt::publish(const char* topic, const uint8_t* payload, unsigned int plength, boolean wait) {
  return publish(topic, payload, plength, false, wait);
}

boolean esp_mqtt::publish(const char* topic, const uint8_t* payload, unsigned int plength, boolean retained, boolean wait) {
  int msg_id = ESP_FAIL;
  if (wait)
    msg_id = esp_mqtt_client_publish(_mqtt_handle, topic, (const char*)payload, plength, 1, retained);
  else
    msg_id = esp_mqtt_client_enqueue(_mqtt_handle, topic, (const char*)payload, plength, 1, retained, true);
  if (msg_id != ESP_FAIL) {
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
  } else {
    ESP_LOGI(TAG, "sent publish unsuccessful, msg_id=%d", msg_id);
  }
  return msg_id != ESP_FAIL;
}

boolean esp_mqtt::publish_P(const char* topic, const char* payload, boolean retained, boolean wait) {
  return publish(topic, (const uint8_t*)payload, 0, retained, wait);
}

boolean esp_mqtt::publish_P(const char* topic, const uint8_t* payload, unsigned int plength, boolean retained, boolean wait) {
  return publish(topic, payload, plength, retained, wait);
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
  if (status_cb != nullptr)
    return this->_state == MQTT_CONNECTED;
  if (connect_called)
    return this->_state == MQTT_CONNECTED;
  return false;
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
  // ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_DATA: {
      ESP_LOGI(TAG, "MQTT_EVENT_DATA");
      char topic_str[event->topic_len];
      memcpy(topic_str, event->topic, event->topic_len);
      message_cb(topic_str, (uint8_t*)event->data, event->data_len);
      break;
    }
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
      this->_state = MQTT_CONNECTED;
      if (status_cb != nullptr)
        status_cb(this->_state);
      break;
    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
      this->_state = MQTT_DISCONNECTED;
      connect_called = false;
      if (status_cb != nullptr)
        status_cb(this->_state);
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
        this->_state = MQTT_CONNECT_FAILED;
        log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
        log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
        log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
        ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
      } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
        // ESP_LOGI(TAG, "Connection refused error: %d", event->error_handle->error_code);
        this->_state = MQTT_CONNECT_FAILED;
      }
      if (status_cb != nullptr)
        status_cb(this->_state);
      break;
    case MQTT_EVENT_BEFORE_CONNECT:
      ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
      break;
    case MQTT_EVENT_DELETED:
      ESP_LOGI(TAG, "MQTT_EVENT_DELETED");
      break;
    default:
      ESP_LOGI(TAG, "Other event id:%d", event->event_id);
      break;
  }
}

size_t esp_mqtt::write(uint8_t) {
  return 0;
}