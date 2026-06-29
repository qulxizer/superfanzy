#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"

/**
 * @brief Log the list of Wi-Fi access points found during a scan.
 *
 * @param tag        The logging tag to use for ESP_LOGI.
 * @param ap_records Pointer to an array of wifi_ap_record_t structures
 * containing the access point information.
 * @param ap_count   The number of access points found (size of the ap_records
 * array).
 */
void sf_log_records(const char *tag, wifi_ap_record_t *ap_records,
                    int ap_count) {
  ESP_LOGI(tag, "Found %d Access Points:", ap_count);
  for (int i = 0; i < ap_count; i++) {
    ESP_LOGI(tag, "SSID: %-32s | RSSI: %d dBm | Channel: %d",
             ap_records[i].ssid, ap_records[i].rssi, ap_records[i].primary);
  }
}

esp_netif_t *sf_wifi_init_netif(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  return esp_netif_create_default_wifi_sta();
}

/**
 * @brief Initialize Wi-Fi with the specified mode (STA, AP, or both)
 * @param mode The Wi-Fi mode to initialize (WIFI_MODE_STA, WIFI_MODE_AP, or
 * WIFI_MODE_APSTA).
 * @return ESP_OK on success, or an error code on failure.
 **/
esp_err_t sf_init_wifi(wifi_mode_t mode) {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(mode));
  return esp_wifi_start();
}
