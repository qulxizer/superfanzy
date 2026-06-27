// Loading WiFi credentials from sdkconfig
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "scan.h"
#include "soc/gpio_num.h"
#include "util.h"

#define SSID CONFIG_WIFI_SSID
#define PASSWD CONFIG_WIFI_PASSWORD

#define SCAN_METHOD WIFI_FAST_SCAN
#define SCAN_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL

#define RSSI -127
#define AUTHMODE WIFI_AUTH_OPEN
#define RSSI_5G_ADJUSTMENT 0

#define MAX_SCAN_RECORDS 20
static const char *TAG = "scan";

// static void event_handler(void *arg, esp_event_base_t event_base,
//                           int32_t event_id, void *event_data) {
//   if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
//     esp_wifi_connect();
//   } else if (event_base == WIFI_EVENT &&
//              event_id == WIFI_EVENT_STA_DISCONNECTED) {
//     esp_wifi_connect();
//   } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
//     ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
//     gpio_set_level(GPIO_NUM_2, 1);
//     ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
//   }
// }

int app_main(void) {
  // Configure GPIO2 as output for LED control
  gpio_config_t io_conf_out = {.pin_bit_mask = (1ULL << GPIO_NUM_2),
                               .mode = GPIO_MODE_OUTPUT,
                               .pull_up_en = GPIO_PULLUP_DISABLE,
                               .pull_down_en = GPIO_PULLDOWN_DISABLE,
                               .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&io_conf_out);
  gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
  gpio_set_level(GPIO_NUM_2, 0);

  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  wifi_ap_record_t *ap_records = NULL;

  size_t ap_count = fast_scan(MAX_SCAN_RECORDS, TAG, &ap_records);
  log_records(TAG, ap_records, ap_count);

  free(ap_records);

  return 0;
}
