#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H
#include "esp_wifi_types_generic.h"

void log_records(const char *tag, wifi_ap_record_t *ap_records, int ap_count);
#endif // UTILS_H
