idf_component_register(SRCS "sdcard.c"
                       INCLUDE_DIRS include
                       REQUIRES sdmmc
                       PRIV_REQUIRES esp_driver_sdmmc board)
