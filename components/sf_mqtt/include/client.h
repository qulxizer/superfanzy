#ifndef SF_MQTT_CLIENT
#define SF_MQTT_CLIENT
#include <stdbool.h>

void sf_mqtt_start(void);

int sf_mqtt_subscribe(const char *topic, int qos);

int sf_mqtt_publish(const char *topic, const char *payload, int qos,
                    bool retain);

void sf_mqtt_client_start(void);

#endif // !SF_MQTT_CLIENT
