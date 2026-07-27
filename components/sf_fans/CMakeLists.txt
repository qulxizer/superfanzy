idf_component_register(SRCS "fans.c"
                    REQUIRES cjson esp_wifi nvs_flash esp_driver_ledc esp_driver_gpio esp_pm
                    INCLUDE_DIRS "." "include")

