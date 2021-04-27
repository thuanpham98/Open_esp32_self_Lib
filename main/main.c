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
#include "driver/spi_master.h"
#include "driver/timer.h"
#include "driver/spi_master.h"

static char *TAG="SPI_TEST";

#define DMA_CHAN    1

#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   15

#define READ_BIT 0x80

spi_device_handle_t mpu9250_spi_handle;
uint8_t receive_data[5];

//This function is called with every interrupt
// static void IRAM_ATTR mpu_isr(void *discard)
// {
//   spi_transaction_t t = {};
//   uint8_t tx_buffer[2] = {0x00, 0x00};
//   t.length = 2 * 8;
//   t.flags = SPI_TRANS_USE_RXDATA; 
//   t.tx_buffer = tx_buffer;
//   t.cmd = MPUREG_ACCEL_XOUT_H | READ_FLAG;                                      
//   spi_device_queue_trans(mpu9250_spi_handle, &t, 0);
// }

// void task_mpu9250(void *pvParameters)
// {
//   // Create a queue capable of containing 20 int16 values.
//   data_queue = xQueueCreate(20, sizeof(int16_t));

//   mpu9250_init_spi();
//   // Check if connection to Sensor is ok
//   if (mpu9250_whoami())
//   {
//     mpu9250_init();
//     gpio_set_intr_type(PIN_MPU_INT, GPIO_INTR_NEGEDGE); 
//     gpio_install_isr_service(0);
//     gpio_isr_handler_add(PIN_MPU_INT, mpu_isr, (void *)PIN_MPU_INT);
//   }
//   vTaskDelete(NULL);
// }

// This is called as a post SPI transaction 
uint8_t flad =0;
void mpu9250_post_spi_trans()
{
    // int16_t data_buffer;
    // data_buffer = ((int16_t)t->rx_data[0] << 8) | t->rx_data[1];
    // ESP_LOGI(TAG,"%d",data_buffer);
    ESP_LOGI(TAG,"DONE send");
    ESP_LOGI(TAG,"%d",receive_data[0]);
    flad=1;

}

void mpu9250_init_spi()
{
  esp_err_t ret;
  spi_bus_config_t buscfg = {.miso_io_num = PIN_NUM_MISO,
                             .mosi_io_num = PIN_NUM_MOSI,
                             .sclk_io_num = PIN_NUM_CLK,
                             .quadwp_io_num = -1,
                             .quadhd_io_num = -1};
  spi_device_interface_config_t devcfg = {
      .address_bits =0,
      .command_bits = 8,                
      .dummy_bits = 0,                  
      .mode = 3,       
      .duty_cycle_pos=0,
      .cs_ena_posttrans=0,
      .cs_ena_pretrans =0,
      .flags=0,                 
      .clock_speed_hz = 1000000,       
      .spics_io_num = PIN_NUM_CS,     
      .queue_size = 1,  
      .pre_cb = NULL,               
      .post_cb = NULL ,//mpu9250_post_spi_trans ,
  };
                                       
  ret = spi_bus_initialize(HSPI_HOST, &buscfg, DMA_CHAN); 
  assert(ret == ESP_OK);
  ESP_LOGI(TAG, "... Initializing bus.");
  ret = spi_bus_add_device(HSPI_HOST, &devcfg, &mpu9250_spi_handle);
  assert(ret == ESP_OK);
  ESP_LOGI(TAG, "... Adding device bus.");
}

esp_err_t mpu9250_write_spi_8bit(uint8_t addr , uint8_t data){
  esp_err_t ret;
  uint8_t tx_data[1] ;
  spi_transaction_t trans_desc;
  memset(&trans_desc, 0, sizeof(trans_desc));
  
  trans_desc.addr = 0;
  trans_desc.cmd = 0;
  trans_desc.flags  = 0;
  trans_desc.length = 8; // total data bits
  trans_desc.tx_buffer = tx_data;
  tx_data[0] = addr;
  // tx_data[1] = data;
  ret= spi_device_queue_trans(mpu9250_spi_handle, &trans_desc, portMAX_DELAY);
  return ret ;
}
esp_err_t mpu9250_read_spi_8bit(uint8_t addr , uint8_t data_size){
  esp_err_t ret;
  uint8_t tx_data[2] ;
  spi_transaction_t trans_desc;
  memset(&trans_desc, 0, sizeof(trans_desc));
  
  trans_desc.addr = 0;
  trans_desc.cmd = 0;
  trans_desc.flags  = 0;
  trans_desc.length = 8 * 2 + 8*data_size; // total data bits
  trans_desc.tx_buffer = tx_data;
  trans_desc.rxlength =8*data_size;
  trans_desc.rx_buffer = receive_data;
  tx_data[0] = 12;
  tx_data[1] = 14;
  ret= spi_device_queue_trans(mpu9250_spi_handle, &trans_desc, portMAX_DELAY);
  return ret ;
}

void app_main(void)
{
    mpu9250_init_spi();
    
    vTaskDelay(5/portTICK_PERIOD_MS);
    while(1){
      mpu9250_write_spi_8bit(14 , 13);
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}
