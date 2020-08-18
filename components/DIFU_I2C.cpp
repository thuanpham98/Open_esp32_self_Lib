#include "DIFU_I2C.h"

DIFU_I2C::DIFU_I2C(uint8_t i2c_mode, uint8_t i2c_port,int8_t pin_sda, int8_t pin_scl)
{
    mode=i2c_mode;
    port=i2c_port;
    sda_pin=pin_sda;
    scl_pin=pin_scl;
}

esp_err_t DIFU_I2C::i2c_initial()
{
    return 1;
}

esp_err_t DIFU_I2C::i2c_master_write()
{
    return 2;
}

DIFU_I2C::~DIFU_I2C()
{
    ESP_LOGE(TAG_I2C,"instane is delete");
}
