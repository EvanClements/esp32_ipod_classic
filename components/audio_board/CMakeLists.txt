# Shadows ESP-ADF's audio_board component (project components win name
# conflicts): this project is a custom board, so we provide the board layer
# ourselves — pin lookups for ADF streams plus the board.h that
# esp_peripherals expects to exist.
idf_component_register(SRCS "board_pins_config.c"
                       INCLUDE_DIRS include
                       REQUIRES driver
                       PRIV_REQUIRES board audio_sal)
