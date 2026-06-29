#ifndef SF_WIFI_UTILS_H
#define SF_WIFI_UTILS_H
#include "esp_err.h"
#include "esp_wifi_types_generic.h"

void sf_wifi_init_netif(void);
esp_err_t sf_init_wifi(wifi_mode_t mode);
void sf_log_records(const char *tag, wifi_ap_record_t *ap_records,
                    int ap_count);

#endif // SF_UTILS_H
