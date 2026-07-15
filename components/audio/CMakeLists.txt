idf_component_register(SRCS "audio.c"
                       INCLUDE_DIRS include
                       PRIV_REQUIRES audio_pipeline audio_stream esp-adf-libs audio_sal
                                     esp_driver_i2s board)
