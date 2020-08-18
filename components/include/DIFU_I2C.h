#ifndef __DIFU_I2C_H
#define __DIFU_I2C_H

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"

#define I2C_MASTER_FREQ_HZ_STANDARD_MODE 100000
#define I2C_MASTER_FREQ_HZ_FAST_MODE 400000

#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define WRITE_BIT I2C_MASTER_WRITE
#define READ_BIT I2C_MASTER_READ
#define ACK_CHECK_EN I2C_MASTER_NACK
#define ACK_CHECK_DIS I2C_MASTER_ACK
#define ACK_VAL I2C_MASTER_ACK
#define NACK_VAL I2C_MASTER_NACK

#define I2C_MASTER_NUM_STANDARD_MODE I2C_NUM_0
#define I2C_MASTER_NUM_FAST_MODE I2C_NUM_1
const char TAG_I2C[]="i2c";
class DIFU_I2C
{
private:
    uint8_t mode;
    uint8_t port;
    int8_t sda_pin;
    int8_t scl_pin;
public:
    DIFU_I2C(uint8_t i2c_mode, uint8_t i2c_port,int8_t pin_sda, int8_t pin_scl);
    ~DIFU_I2C();
    esp_err_t i2c_initial();
    esp_err_t i2c_master_write();
};

#endif