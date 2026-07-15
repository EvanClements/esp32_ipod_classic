idf_component_register(SRCS "app_main.c" "console.c"
                       PRIV_REQUIRES console nvs_flash sdcard audio usb_msc ota fatfs)
