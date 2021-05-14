#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_log.h>
#include <esp_err.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/uart.h"

static char *TAG="I2S_TEST";
DMA_ATTR uint8_t buffer[20];
DMA_ATTR uint8_t data[]="hello";

void send(void *pv){
  while (1)
  {
    // for(int i =0 ; i < 5; i++){
      uart_write_bytes(UART_NUM_1, (const char *)data, 5);
    // }
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
  
}

void receive(void *pv){
  while (1)
  {
    int length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_2, (size_t*)&length));
    length = uart_read_bytes(UART_NUM_2, buffer, length, portMAX_DELAY);
    // ESP_LOGW(TAG,"%d",length);
    if(length >0){
      // for(int i = 0; i < length ; i++){
        ESP_LOGI(TAG,"%s",buffer);
      // }
    }
    memset(buffer,0,20);
    
    vTaskDelay(100);
  }
  vTaskDelete(NULL);
  
}

void app_main(void)
{

  uart_config_t uart_tx_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
  };
  // Configure UART parameters
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_tx_config));

  // Set UART pins(TX: IO17 (UART2 default), RX: IO16 (UART2 default), RTS: IO18, CTS: IO19)
  ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, 21, 22, -1, -1));

  // Setup UART buffered IO with event queue
  QueueHandle_t uart_tx_queue;
  // Install UART driver using an event queue here
  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 1024/2, \
                                          1024/2, 0, NULL, 0));

// -------------------------------------------------------------------------
  uart_config_t uart_rx_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
  };
  // Configure UART parameters
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_rx_config));

  // Set UART pins(TX: IO17 (UART2 default), RX: IO16 (UART2 default), RTS: IO18, CTS: IO19)
  ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 25, 26, -1, -1));

  // Setup UART buffered IO with event queue
  QueueHandle_t uart_rx_queue;
  // Install UART driver using an event queue here
  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, 1024/2, \
                                          1024/2, 1, NULL, 0));

  xTaskCreate(send,"send",4096,NULL,2,NULL);
  xTaskCreate(receive,"receive",4096,NULL,3,NULL);

}
