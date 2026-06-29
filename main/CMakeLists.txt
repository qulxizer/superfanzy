idf_component_register(SRCS "main.c"
                    PRIV_REQUIRES esp_wifi nvs_flash esp_driver_gpio sf_wifi sf_mqtt
                    INCLUDE_DIRS ".")
