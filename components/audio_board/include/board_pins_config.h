/*
 * Board pin lookups used by ESP-ADF streams (same interface as the header in
 * $ADF_PATH/components/audio_board/include/board_pins_config.h, which this
 * component shadows).
 */
#ifndef _BOARD_PINS_CONFIG_H_
#define _BOARD_PINS_CONFIG_H_

#include "driver/i2c.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int mck_io_num;   /*!< MCK pin, output */
    int bck_io_num;   /*!< BCK pin, input in slave role, output in master role */
    int ws_io_num;    /*!< WS pin, input in slave role, output in master role */
    int data_out_num; /*!< DATA pin, output */
    int data_in_num;  /*!< DATA pin, input */
} board_i2s_pin_t;

esp_err_t get_i2c_pins(i2c_port_t port, i2c_config_t *i2c_config);

esp_err_t get_i2s_pins(int port, board_i2s_pin_t *i2s_config);

esp_err_t get_spi_pins(spi_bus_config_t *spi_config,
                       spi_device_interface_config_t *spi_device_interface_config);

/* Referenced at compile time by ADF's audio_hal codec drivers (es8388 etc.).
 * None of those codecs exist on this board — all return "not present" (-1). */
int8_t get_pa_enable_gpio(void);
int8_t get_es7243_mclk_gpio(void);
int8_t get_headphone_detect_gpio(void);
int8_t get_reset_board_gpio(void);

#ifdef __cplusplus
}
#endif

#endif /* _BOARD_PINS_CONFIG_H_ */
