idf_component_register(SRCS "util.c" "scan.c"
                    PRIV_REQUIRES esp_wifi nvs_flash 
                    INCLUDE_DIRS "include")

