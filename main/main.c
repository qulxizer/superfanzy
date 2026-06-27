// Loading WiFi credentials from sdkconfig
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "soc/gpio_num.h"

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

static void fast_scan(void) {
  // 2. Initialize Netif and Event Loop
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  // 3. Initialize Wi-Fi with Default Configurations
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());

  // 4. Configure Scan Parameters (Optional: NULL uses defaults)
  wifi_scan_config_t scan_config = {.ssid = NULL,
                                    .bssid = NULL,
                                    .channel = 0,
                                    .show_hidden = true,
                                    .scan_type = WIFI_SCAN_TYPE_ACTIVE};

  // 5. Start Scan (True blocks execution until scan completes)
  ESP_LOGI(TAG, "Starting Wi-Fi scan...");
  ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

  // 6. Allocate Memory and Fetch AP Records
  uint16_t ap_count = 0;
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

  if (ap_count > MAX_SCAN_RECORDS) {
    ap_count = MAX_SCAN_RECORDS;
  }

  wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
  if (ap_records == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for AP records");
    return;
  }

  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_records));

  // 7. Loop Through and Print Discovered AP Names
  ESP_LOGI(TAG, "Found %d Access Points:", ap_count);
  for (int i = 0; i < ap_count; i++) {
    ESP_LOGI(TAG, "SSID: %-32s | RSSI: %d dBm | Channel: %d",
             ap_records[i].ssid, ap_records[i].rssi, ap_records[i].primary);
  }

  // Clean up allocated buffer
  free(ap_records);
}

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
  fast_scan();

  return 0;
}
