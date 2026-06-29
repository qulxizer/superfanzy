#ifndef SF_MQTT_CLIENT
#define SF_MQTT_CLIENT
#include <stdbool.h>

typedef void (*sf_mqtt_msg_cb_t)(const char *topic, const char *payload);

void sf_mqtt_start(sf_mqtt_msg_cb_t msg_cb);

int sf_mqtt_subscribe(const char *topic, int qos);

int sf_mqtt_publish(const char *topic, const char *payload, int qos,
                    bool retain);

void sf_mqtt_client_start(void);

#endif // !SF_MQTT_CLIENT
