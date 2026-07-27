#include "cJSON.h"
#include "esp_log.h"
#include "fans.h"

#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "mqtt_client.h"
#include "sdkconfig.h"

#include "client.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "SF_MQTT";

static esp_mqtt_client_handle_t s_client;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data);

void sf_mqtt_start(void) {
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
static void get_esp_serial_string(char *serial_str, size_t str_size) {
  uint8_t mac[6];

  if (esp_efuse_mac_get_default(mac) == ESP_OK) {
    // Format as uppercase hex digits separated by colons
    snprintf(serial_str, str_size, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],
             mac[1], mac[2], mac[3], mac[4], mac[5]);

    // ESP_LOGI(TAG, "ESP Serial Number (MAC): %s", serial_str);
  } else {
    ESP_LOGE(TAG, "Failed to read default MAC address");
  }
}

uint32_t get_uptime_seconds(void) {
  // Get total ticks elapsed since boot
  TickType_t total_ticks = xTaskGetTickCount();

  // Convert ticks to seconds
  return (uint32_t)(total_ticks / configTICK_RATE_HZ);
}

static void sf_mqtt_publisher_task(void *pvParameters) {
  ESP_LOGI(TAG, "MQTT_PUB_TASK");
  while (1) {
    if (s_client != NULL) {
      vTaskDelay(pdMS_TO_TICKS(5000));
      cJSON *obj = cJSON_CreateObject();
      if (obj == NULL) {
        ESP_LOGE(TAG, "Failed to create json object");
        continue;
        ;
      }
      char base_mac[18];
      get_esp_serial_string(base_mac, sizeof(base_mac));
      cJSON_AddStringToObject(obj, "device_mac", base_mac);
      cJSON_AddStringToObject(obj, "status", "online");
      cJSON_AddNumberToObject(obj, "uptime_s", get_uptime_seconds());

      wifi_ap_record_t ap_info;
      esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get AP info: %s", esp_err_to_name(err));
        continue;
      }

      char ssid_str[33];
      snprintf(ssid_str, sizeof(ssid_str), "%s", (char *)ap_info.ssid);
      cJSON_AddStringToObject(obj, "wifi_ssid", (char *)ssid_str);
      char *printed = cJSON_PrintUnformatted(obj);
      cJSON_Delete(obj);
      if (printed == NULL) {
        ESP_LOGE(TAG, "Failed to print cjson object");
        continue;
      }
      sf_mqtt_publish("/fanctl/status", printed, 1, false);
      cJSON_free(printed);
    }
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
    xTaskCreate(sf_mqtt_publisher_task, "mqtt_pub_task", 3072, NULL, 5, NULL);

    esp_mqtt_topic_t topics[] = {
        (esp_mqtt_topic_t){.filter = "/fanctl/control/fan/1/PWM", .qos = 0},
        (esp_mqtt_topic_t){.filter = "/fanctl/control/fan/2/PWM", .qos = 0},
        (esp_mqtt_topic_t){.filter = "/fanctl/control/fan/3/PWM", .qos = 0},
        (esp_mqtt_topic_t){.filter = "/fanctl/control/fan/4/PWM", .qos = 0},
    };
    msg_id = esp_mqtt_client_subscribe_multiple(
        client, topics, sizeof(topics) / sizeof(topics[0]));
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
    sf_fan_t fan = {0};
    sf_fans_get_fan(event->topic, event->topic_len, &fan);
    // Cap the length just in case so we don't overflow the buffer
    int len = event->data_len;
    if (len > 15)
      len = 15;

    // Create a temporary buffer and copy the data
    char data_str[16];
    memcpy(data_str, event->data, len);
    data_str[len] = '\0'; // Null-terminate it manually

    int duty = atoi(data_str);

    const int max_duty = (1 << fan.tim.ledc_duty_res) - 1;
    if (duty > max_duty)
      duty = 1023;
    sf_fans_set_duty(&fan, duty);
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

// void sf_mqtt_client_start(void) {
//   esp_mqtt_client_config_t mqtt_cfg = {.broker.address.uri =
//                                            CONFIG_SF_MQTT_BROKER_URI
//
//   };
//
//   esp_mqtt_client_handle_t s_client = esp_mqtt_client_init(&mqtt_cfg);
//   esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID,
//   mqtt_event_handler,
//                                  NULL);
//   esp_mqtt_client_start(s_client);
// }
