// Loading WiFi credentials from sdkconfig
#include "client.h"
#include "connect.h"
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

  sf_wifi_init_netif();
  sf_init_wifi(WIFI_MODE_STA);
  // wifi_ap_record_t *ap_records = NULL;
  // size_t ap_count = sf_fast_scan(MAX_SCAN_RECORDS, TAG, &ap_records);
  // sf_log_records(TAG, ap_records, ap_count);
  // free(ap_records);
  sf_connect_to_wifi(SSID, PASSWD);
  sf_mqtt_client_start();

  return 0;
}
