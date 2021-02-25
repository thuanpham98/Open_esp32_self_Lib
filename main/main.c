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

static char *TAG="SPI_TEST";

#define SPI2    HSPI_HOST
#define DMA_CHAN    2

#define PIN_NUM_MISO 27
#define PIN_NUM_MOSI 26
#define PIN_NUM_CLK  25
#define PIN_NUM_CS   5

#define READ_BIT 0x80
#define WRITE_BIT 0x7F
#define SLAVE_ADDR 0x68

#define USER_CTRL 0x6A

spi_device_handle_t spi;
// void test_spi(void *pv);
// void config_MPU9520(spi_device_handle_t _spi);

// void send_cmd(spi_device_handle_t _spi, const uint8_t cmd)
// {
//     esp_err_t ret;
//     spi_transaction_t t;
    
//     spi_transaction_t *_receive;
//     uint8_t data_receive[5];
//     memset(&t, 0, sizeof(t));
//     t.length=8;
//     t.tx_buffer=&cmd;
//     // t.rxlength =1;
//     // t.rx_buffer = data_receive;
//     t.flags = SPI_TRANS_USE_RXDATA;

//     ESP_ERROR_CHECK(spi_device_queue_trans(spi, &t, portMAX_DELAY));
//     ESP_LOGI(TAG,"%d",*(uint32_t*)t.rx_data);

//     ESP_ERROR_CHECK(spi_device_get_trans_result(spi, &_receive, portMAX_DELAY));
// ESP_LOGI(TAG,"%d",t.rx_data[0]);
 
// }

// void config_MPU9520(spi_device_handle_t _spi){
//     esp_err_t ret;
    // spi_transaction_t _transmit;
    // spi_transaction_t *_receive=NULL;
//     uint8_t receidata[5];

//     send_cmd(spi,0x68);

    // memset(&_transmit, 0, sizeof(_transmit));
    // _transmit.length=8;
    // _transmit.flags = SPI_TRANS_USE_RXDATA;
    // _transmit.user = (void*)1;
    // _transmit.rxlength = 8;
    // *(uint8_t *)(_transmit.tx_buffer)=(0x80|0x68); 

    // ESP_ERROR_CHECK(spi_device_queue_trans(spi, &_transmit, portMAX_DELAY));
    // ESP_ERROR_CHECK(spi_device_get_trans_result(spi, &_receive, portMAX_DELAY));
    // assert( ret == ESP_OK );

    // ESP_LOGI(TAG,"%d",_transmit.rx_data[0]);

    // return *(uint32_t*)t.rx_data;

    // ping Who Am I 
    // memset(&_transmit, 0, sizeof(_transmit));       //Zero out the transaction
    // memset(_receive, 0, sizeof(_receive));
    // _transmit.length=8;                     //Command is 8 bits
    // _transmit.tx_data[0]=0x68 || 0x80;   
    // _transmit.flags=SPI_TRANS_USE_TXDATA  | SPI_TRANS_USE_RXDATA;             //The data is the cmd itself
    // ESP_ERROR_CHECK(spi_device_queue_trans(spi, &_transmit, portMAX_DELAY));  //Transmit!
    // vTaskDelay(10/portTICK_PERIOD_MS);

    // _receive->flags=SPI_TRANS_USE_RXDATA;
    // _receive->rx_buffer = receidata;
    // ESP_ERROR_CHECK(spi_device_get_trans_result(spi, &_receive, portMAX_DELAY));

    // ESP_LOGI(TAG,"%d", _receive->rx_data[0]);

    // disable I2c mode
    // memset(&_transmit, 0, sizeof(_transmit));       //Zero out the transaction
    // _transmit.length=8;                     //Command is 8 bits
    // _transmit.tx_data[0]=0x10;   
    // _transmit.flags=SPI_TRANS_USE_TXDATA;            //The data is the cmd itself
    // ESP_ERROR_CHECK(spi_device_queue_trans(spi, &_transmit, portMAX_DELAY));  //Transmit!

// }

// void test_spi(void *pv){
//     esp_err_t ret;
//     spi_transaction_t t;
//     spi_transaction_t *rtrans =NULL;

//     config_MPU9520(spi);

//     while(1){
//         ESP_LOGI(TAG,"ok");
//         vTaskDelay(1000/portTICK_RATE_MS);
//     }
// }

void app_main(void)
{
    esp_err_t ret;
    spi_bus_config_t buscfg={0};
    buscfg.miso_io_num=PIN_NUM_MISO;
    buscfg.mosi_io_num=PIN_NUM_MOSI;
    buscfg.sclk_io_num=PIN_NUM_CLK;
    buscfg.quadwp_io_num=-1;
    buscfg.quadhd_io_num=-1;
    buscfg.max_transfer_sz=4096;
    
    spi_device_interface_config_t devcfg={0};
    devcfg.clock_speed_hz=1*1000*1000;
    devcfg.mode=0;                        // cuc dong bo 3 pha , CPOL $ CPHA  
    devcfg.spics_io_num=PIN_NUM_CS;              
    devcfg.queue_size=10;            
    // devcfg.pre_cb=lcd_spi_pre_transfer_callback,

    //Initialize the SPI bus
    ret=spi_bus_initialize(SPI2, &buscfg,0);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(SPI2, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    spi_transaction_t _transmit;
    spi_transaction_t _t;
    spi_transaction_t *_receive;
    uint8_t data[5];
        // ping Who Am I 
    memset(&_transmit, 0, sizeof(_transmit));       //Zero out the transaction
    memset(&_t, 0, sizeof(_t));
    // memset(_receive, 0, sizeof(_receive));
    _transmit.length=8;                     //Command is 8 bits
    // _transmit.rxlength=8;
    data[0]=0x75 || 0x80;
    _transmit.tx_buffer=&data[0];   

    ret=spi_device_polling_transmit(spi, &_transmit);  //Transmit!
    assert(ret==ESP_OK);
    
    _t.length=8*3;
    _t.flags = SPI_TRANS_USE_RXDATA;
    // _t.user = (void*)1;

    ret = spi_device_polling_transmit(spi, &_t);
    assert( ret == ESP_OK );

    ESP_LOGI(TAG,"%d",*(uint32_t*)_t.rx_data);


}
