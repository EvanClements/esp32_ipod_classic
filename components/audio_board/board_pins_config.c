#include "board.h"

#include "board_pins.h"
#include "esp_err.h"

esp_err_t get_i2s_pins(int port, board_i2s_pin_t *i2s_config)
{
    if (i2s_config == NULL || port != 0) {
        return ESP_FAIL;
    }
    i2s_config->mck_io_num = -1; /* PCM5102A runs from its internal PLL, SCK tied to GND */
    i2s_config->bck_io_num = PIN_I2S_BCK;
    i2s_config->ws_io_num = PIN_I2S_LRCK;
    i2s_config->data_out_num = PIN_I2S_DOUT;
    i2s_config->data_in_num = -1;
    return ESP_OK;
}

/* No I2C or SPI peripherals on the ADF board layer for this hardware. */
esp_err_t get_i2c_pins(i2c_port_t port, i2c_config_t *i2c_config)
{
    (void)port;
    (void)i2c_config;
    return ESP_FAIL;
}

esp_err_t get_spi_pins(spi_bus_config_t *spi_config,
                       spi_device_interface_config_t *spi_device_interface_config)
{
    (void)spi_config;
    (void)spi_device_interface_config;
    return ESP_FAIL;
}

int8_t get_pa_enable_gpio(void)
{
    return -1;
}

int8_t get_es7243_mclk_gpio(void)
{
    return -1;
}

int8_t get_headphone_detect_gpio(void)
{
    return -1;
}

int8_t get_reset_board_gpio(void)
{
    return -1;
}

int8_t get_sdcard_open_file_num_max(void)
{
    return 5;
}
