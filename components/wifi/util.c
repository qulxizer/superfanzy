#include "esp_log.h"
#include "esp_wifi_types_generic.h"
void log_records(const char *tag, wifi_ap_record_t *ap_records, int ap_count) {
  ESP_LOGI(tag, "Found %d Access Points:", ap_count);
  for (int i = 0; i < ap_count; i++) {
    ESP_LOGI(tag, "SSID: %-32s | RSSI: %d dBm | Channel: %d",
             ap_records[i].ssid, ap_records[i].rssi, ap_records[i].primary);
  }
}
