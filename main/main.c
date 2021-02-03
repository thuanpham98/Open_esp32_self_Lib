/* This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this software is
   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

#include "esp_hidh.h"
#include "esp_hid_gap.h"
#include "esp_hid_button.h"

static const char *TAG = "ESP_HIDH_DEMO";
esp_event_base_t TEST ;
ESP_EVENT_DECLARE_BASE(TEST);
ESP_EVENT_DEFINE_BASE(TEST);

// 1. Define the event handler
void run_on_event(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data)
{   
   uint8_t *my_data = (uint8_t *)event_data;
   
   if(base==TEST){
       if(id==1){
           ESP_LOGI(TAG,"this is id 1");
           ESP_LOGI(TAG,"%d %d %d %d %d",*(my_data),*(my_data+1),*(my_data+2),*(my_data+3),*(my_data+4));
       }else if(id==2){
           ESP_LOGI(TAG,"this is id 2");
           ESP_LOGI(TAG,"%d %d %d %d %d",*(my_data),*(my_data+1),*(my_data+2),*(my_data+3),*(my_data+4));
       }
   }
}

uint8_t data[5]={1,2,3,4,5};
uint8_t data2[5]={6,7,8,9,10};

void app_main()
{
    esp_event_loop_args_t loop_args = {
        .queue_size = 512,
        .task_name = "myEvent",
        .task_priority = 3,
        .task_stack_size = 2048,
        .task_core_id = 1
    };

    esp_event_loop_handle_t loop_handle;

    esp_event_loop_create(&loop_args, &loop_handle);

    esp_event_handler_register_with(loop_handle, "TEST", 1, run_on_event, data);
    esp_event_handler_register_with(loop_handle, "TEST", 2, run_on_event, data2);
    
    int32_t id1=1;
    int32_t id2=2;
    while (1)
    {
        esp_event_post_to(loop_handle, TEST, id1,data , sizeof(data),portMAX_DELAY);
        vTaskDelay(1000/portTICK_RATE_MS);
        esp_event_post_to(loop_handle, TEST, id2, data2,sizeof(data2),portMAX_DELAY);
        vTaskDelay(1000/portTICK_RATE_MS);
    }

    // esp_event_handler_unregister_with(loop_handle, TEST, 1, run_on_event);  we can call esp_event_post_to but loop so not anything
    vTaskDelay(1000/portTICK_RATE_MS);
    esp_event_loop_delete(loop_handle); // after use this function , we can not call esp_event_post_to againe
    
}
