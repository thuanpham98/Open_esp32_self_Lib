#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "DIFU_I2C.h"
#include "driver/gpio.h"
#include <esp_log.h>
#include <esp_err.h>

// #define PIN_NUM_MISO 18 
// #define PIN_NUM_MOSI 23
// #define PIN_NUM_CLK  19
// #define PIN_NUM_CS   13

extern "C" {
    void app_main(void);
    // void _spi1_init_bus(void);
    // void _spi1_transmit(spi_device_handle_t handle_spi,uint8_t *data);
}
// uint8_t data_rev[32];
// uint8_t data_send[32];
static const char TAG[]="esp32_nek";
// spi_device_handle_t spi;
//--------------------------------------------//

// static void cs_high(spi_transaction_t* t)
// {
//     ESP_LOGI(TAG, "cs high");
//     gpio_set_level(GPIO_NUM_13,1);
// }

// static void cs_low(spi_transaction_t* t)
// {
//     ESP_LOGI(TAG, "cs low");
//     gpio_set_level(GPIO_NUM_13,0);
// }

// void _spi1_init_bus()
// {
//     /* config bus */
//     spi_bus_config_t bus_cf={0};
//     bus_cf.miso_io_num=PIN_NUM_MISO;
//     bus_cf.mosi_io_num=PIN_NUM_MOSI;
//     bus_cf.sclk_io_num=PIN_NUM_CLK;
//     bus_cf.quadwp_io_num = -1;
//     bus_cf.quadhd_io_num = -1;
//     bus_cf.max_transfer_sz = 32;

//     esp_err_t ret = spi_bus_initialize(SPI1_HOST,&bus_cf,2);
//     ESP_ERROR_CHECK(ret);
//     ESP_LOGE(TAG,"%s",esp_err_to_name(ret));

//     /* config device connected */
//     spi_device_interface_config_t devcf={0};
//     devcf.clock_speed_hz=1000000;
//     devcf.spics_io_num=PIN_NUM_CS;
//     devcf.queue_size=1;
//     devcf.mode=0;
//     devcf.input_delay_ns=500;
//     devcf.flags =SPI_DEVICE_POSITIVE_CS;
//     devcf.pre_cb=cs_high;
//     devcf.post_cb=cs_low;

//     ESP_ERROR_CHECK(spi_bus_add_device(SPI1_HOST,&devcf,&spi));
// }
// void _spi1_transmit(spi_device_handle_t handle_spi,uint8_t *data)
// {
//     spi_transaction_t t={0};
//     memset(&t, 0, sizeof(t));     
//     t.length=4*8;                
//     t.tx_buffer=data;               
//     t.user=(void*)1; 
//     esp_err_t err = spi_device_polling_transmit(handle_spi, &t);
// }
void app_main(void)
{
    // gpio_pad_select_gpio(GPIO_NUM_13);
    // gpio_set_direction(GPIO_NUM_13,GPIO_MODE_OUTPUT);
    // _spi1_init_bus();
    ESP_LOGE(TAG,"%d",esp_get_free_heap_size());
    DIFU_I2C *i2c_nek= new DIFU_I2C(0,1,12,13);
    DIFU_I2C *i2c_nek2= new DIFU_I2C(0,1,12,13);
    ESP_LOGE(TAG,"%d",esp_get_free_heap_size());
    uint8_t ret;

    ret = i2c_nek->i2c_initial();
    ESP_LOGI(TAG,"%d",ret);
    ret=i2c_nek->i2c_master_write();

    ESP_LOGI(TAG,"%d",ret);
    // i2c_nek->~DIFU_I2C();
    // i2c_nek2->~DIFU_I2C();
    ESP_LOGE(TAG,"%d",esp_get_free_heap_size());
    vTaskDelay(1000/portTICK_PERIOD_MS);
    ESP_LOGE(TAG,"%d",esp_get_free_heap_size());

}