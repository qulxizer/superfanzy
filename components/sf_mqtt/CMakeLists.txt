idf_component_register(SRCS "client.c"
                    REQUIRES mqtt esp_wifi nvs_flash 
                    INCLUDE_DIRS "." "include")

