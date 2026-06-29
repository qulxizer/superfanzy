#include "cjson/cJSON.h"
#include "esp_log.h"

#include "mqtt_client.h"
#include "sdkconfig.h"

#include "client.h"

#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "SF_MQTT:";

static esp_mqtt_client_handle_t s_client;
static sf_mqtt_msg_cb_t s_msg_cb;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;

  switch (event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "Connected");
    break;

  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGW(TAG, "Disconnected");
    break;

  case MQTT_EVENT_DATA: {
    char *topic = strndup(event->topic, event->topic_len);
    char *payload = strndup(event->data, event->data_len);

    if (!topic || !payload) {
      free(topic);
      free(payload);
      return;
    }

    if (s_msg_cb) {
      s_msg_cb(topic, payload);
    }

    free(topic);
    free(payload);
    break;
  }

  default:
    break;
  }
}

void sf_mqtt_start(sf_mqtt_msg_cb_t msg_cb) {
  s_msg_cb = msg_cb;

  esp_mqtt_client_config_t config = {
      .broker.address.uri = CONFIG_SF_MQTT_BROKER_URI,
  };

  s_client = esp_mqtt_client_init(&config);

  esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 NULL);

  esp_mqtt_client_start(s_client);
}

int sf_mqtt_subscribe(const char *topic, int qos) {
  if (!s_client) {
    return -1;
  }

  return esp_mqtt_client_subscribe(s_client, topic, qos);
}

int sf_mqtt_publish(const char *topic, const char *payload, int qos,
                    bool retain) {
  if (!s_client) {
    return -1;
  }

  return esp_mqtt_client_publish(s_client, topic, payload, 0, qos, retain);
}

static void log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  ESP_LOGD(TAG,
           "Event dispatched from event loop base=%s, event_id=%" PRIi32 "",
           base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    msg_id =
        esp_mqtt_client_publish(client, "fanctl/status", "data_3", 0, 1, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

    // esp_mqtt_topic_t topics[] = {
    //     (esp_mqtt_topic_t){.filter = "/fans/control/12/PWM", .qos = 0},
    //     (esp_mqtt_topic_t){.filter = "/topic/qos0", .qos = 0},
    // };
    // msg_id = esp_mqtt_client_subscribe_multiple(client, topics, 0);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d, return code=0x%02x ",
             event->msg_id, (uint8_t)*event->data);
    msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
      log_error_if_nonzero("reported from esp-tls",
                           event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack",
                           event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno",
                           event->error_handle->esp_transport_sock_errno);
      ESP_LOGI(TAG, "Last errno string (%s)",
               strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

void sf_mqtt_client_start(void) {
  esp_mqtt_client_config_t mqtt_cfg = {.broker.address.uri =
                                           CONFIG_SF_MQTT_BROKER_URI

  };

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 NULL);
  esp_mqtt_client_start(client);
}
