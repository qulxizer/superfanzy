
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "util.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/**
 * @brief Perform a fast Wi-Fi scan and return discovered AP names.
 *
 * This function handles the network interface initialization, sets the Wi-Fi
 * station mode, and executes an active blocking scan.
 *
 * @note Requires NVS to be initialized before calling this function.
 * @note Network interfaces and Wi-Fi system can only be initialized once
 * globally.
 *
 * @attention The caller is responsible for freeing the allocated memory buffer
 *            returned via `ret_ap_records` by calling `free()` when done.
 *
 * @param[in]  max_scan_records Maximum number of AP records to retrieve.
 * @param[in]  tag              String literal tag used for logging purposes.
 * @param[out] ret_ap_records   Pointer to a buffer pointer where the allocated
 *                              array of records will be stored.
 *
 * @return Number of Access Points found and allocated, or 0 if an error occurs.
 */

uint16_t sf_fast_scan(int max_scan_records, const char *tag,
                      wifi_ap_record_t **ret_ap_records) {

  if (ret_ap_records == NULL) {
    return 0;
  }

  // Initialize Wi-Fi with Default Configurations
  sf_init_wifi(WIFI_MODE_STA);

  // Configure Scan Parameters (Optional: NULL uses defaults)
  wifi_scan_config_t scan_config = {.ssid = NULL,
                                    .bssid = NULL,
                                    .channel = 0,
                                    .show_hidden = true,
                                    .scan_type = WIFI_SCAN_TYPE_ACTIVE};

  // Start Scan (True blocks execution until scan completes)
  ESP_LOGI(tag, "Starting Wi-Fi scan...");
  ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

  // Allocate Memory and Fetch AP Records
  uint16_t ap_count = 0;
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

  if (ap_count > max_scan_records) {
    ap_count = max_scan_records;
  }

  wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
  if (ap_records == NULL) {
    ESP_LOGE(tag, "Failed to allocate memory for AP records");
    return 0;
  }

  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_records));
  *ret_ap_records = ap_records; // Return the AP records to the caller
  return ap_count;
}
