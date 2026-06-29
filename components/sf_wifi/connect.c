#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include <string.h>

/**
 * @brief Event handler for Wi-Fi and IP events.
 *
 * This function handles events related to Wi-Fi connection and disconnection,
 * as well as obtaining an IP address. It logs the relevant information to the
 * console.
 *
 * @param arg User-defined argument (not used in this implementation).
 * @param event_base The base type of the event (e.g., WIFI_EVENT, IP_EVENT).
 * @param event_id The specific ID of the event (e.g.,
 * WIFI_EVENT_STA_CONNECTED).
 * @param event_data Optional data associated with the event.
 */
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {

    ip_event_got_ip_t *event = event_data;
    ESP_LOGI("WiFi", "IP: " IPSTR, IP2STR(&event->ip_info.ip));
  }

  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {

    wifi_event_sta_disconnected_t *disc = event_data;
    ESP_LOGI("WiFi", "Disconnected, reason=%d", disc->reason);
  }
}
/**
 * @brief Connect to a Wi-Fi network using the provided SSID and password.
 *
 * @param ssid The SSID of the Wi-Fi network.
 * @param password The password of the Wi-Fi network.
 */
void sf_connect_to_wifi(const char *ssid, const char *password) {
  wifi_config_t wifi_config = {
      .sta.threshold.authmode = WIFI_AUTH_WPA2_PSK,
  };

  strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
  strncpy((char *)wifi_config.sta.password, password,
          sizeof(wifi_config.sta.password));

  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler,
                             NULL);

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI("Wi-Fi", "Connecting to SSID: %s", ssid);

  ESP_ERROR_CHECK(esp_wifi_connect());
}
