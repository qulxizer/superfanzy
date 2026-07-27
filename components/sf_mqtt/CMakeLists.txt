idf_component_register(SRCS "client.c"
                    REQUIRES mqtt cjson esp_wifi nvs_flash sf_fans
                    INCLUDE_DIRS "." "include")

