#ifndef WIFI_SCAN_H
#define WIFI_SCAN_H

#include "esp_wifi_types_generic.h"
#include <stdint.h>
uint16_t fast_scan(int max_scan_records, const char *tag,
                   wifi_ap_record_t **ret_ap_records);

#endif // WIFI_SCAN_H
