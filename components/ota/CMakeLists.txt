idf_component_register(SRCS "ota.c"
                       INCLUDE_DIRS include
                       PRIV_REQUIRES esp_wifi esp_netif esp_event nvs_flash
                                     esp_https_ota app_update esp_http_client mbedtls)
