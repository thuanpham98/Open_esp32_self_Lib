#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <esp_log.h>
#include <esp_err.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/dac.h"
#include "driver/timer.h"
#include "driver/spi_master.h"

static const char TAG[]="SPI_TEST";

#define SPI2    HSPI_HOST
#define DMA_CHAN    2

#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   15

spi_device_handle_t spi;
extern "C" {
    void app_main(void);
    void test_spi(void *pv);
}

void test_spi(void *pv){
    esp_err_t ret;
    spi_transaction_t t;
    uint8_t te =100;
    while (1)
    {
        memset(&t, 0, sizeof(t));       //Zero out the transaction
        t.length=8;                     //Command is 8 bits
        t.tx_data[0]='12';               //The data is the cmd itself
        t.tx_data[1]='13'; 
        t.tx_data[2]='14'; 
        t.tx_data[3]='15'; 

        ret=spi_device_polling_transmit(spi, &t);  //Transmit!
        assert(ret==ESP_OK);
        ESP_LOGI(TAG,"%d",ret);

        vTaskDelay(1/portTICK_PERIOD_MS);
    }
    
}

void app_main(void)
{
    esp_err_t ret;
    spi_bus_config_t buscfg={0};
    buscfg.miso_io_num=PIN_NUM_MISO;
    buscfg.mosi_io_num=PIN_NUM_MOSI;
    buscfg.sclk_io_num=PIN_NUM_CLK;
    buscfg.quadwp_io_num=-1;
    buscfg.quadhd_io_num=-1;
    buscfg.max_transfer_sz=1000;
    
    spi_device_interface_config_t devcfg={0};
    devcfg.clock_speed_hz=10*1000*1000;
    devcfg.mode=0;                        // cuc dong bo 3 pha , CPOL $ CPHA  
    devcfg.spics_io_num=PIN_NUM_CS;              
    devcfg.queue_size=7;            
    // devcfg.pre_cb=lcd_spi_pre_transfer_callback,

    //Initialize the SPI bus
    ret=spi_bus_initialize(SPI2, &buscfg, DMA_CHAN);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(SPI2, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    xTaskCreate(test_spi,"test_spi",2048,NULL,4,NULL);
}
