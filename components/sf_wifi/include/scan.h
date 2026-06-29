#ifndef SF_WIFI_SCAN_H
#define SF_WIFI_SCAN_H

#include "esp_wifi_types_generic.h"
#include <stdint.h>
uint16_t sf_fast_scan(int max_scan_records, const char *tag,
                      wifi_ap_record_t **ret_ap_records);

#endif // SF_WIFI_SCAN_H
