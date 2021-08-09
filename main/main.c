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

#include "stream.h"

static const char *TAG = "Test Stream";

esp_event_base_t TEST ;
ESP_EVENT_DECLARE_BASE(TEST);
ESP_EVENT_DEFINE_BASE(TEST);
uint8_t data[4]={1,2,4,4};

uint16_t thread_id[4] = {1,2,3,4};

void _on_data(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data)
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
esp_event_loop_handle_t loop_handle;

void app_main(){
    
    stream_config_t cf ={
        .loop_args={
            .queue_size = 512,
            .task_name = "myEvent",
            .task_priority = 3,
            .task_stack_size = 2048,
            .task_core_id = 1,
        },
        .loop_handle= loop_handle,
        .event_name="TEST",
        .num_thead=4,
        .time_out = 100,
        .thread=thread_id,
        .data_event =data,
        .onData=_on_data,
    };
    g_stream_handler_t my_handler= stream_init(&cf);
    stream_add_event(my_handler,1,4);
}

// // 1. Define the event handler
// void run_on_event(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data)
// {   
//    uint8_t *my_data = (uint8_t *)event_data;
   
//    if(base==TEST){
//        if(id==1){
//            ESP_LOGI(TAG,"this is id 1");
//            ESP_LOGI(TAG,"%d %d %d %d %d",*(my_data),*(my_data+1),*(my_data+2),*(my_data+3),*(my_data+4));
//        }else if(id==2){
//            ESP_LOGI(TAG,"this is id 2");
//            ESP_LOGI(TAG,"%d %d %d %d %d",*(my_data),*(my_data+1),*(my_data+2),*(my_data+3),*(my_data+4));
//        }
//    }
// }

// uint8_t data[5]={1,2,3,4,5};
// uint8_t data2[5]={6,7,8,9,10};

// void app_main()
// {
//     esp_event_loop_args_t loop_args = {
//         .queue_size = 512,
//         .task_name = "myEvent",
//         .task_priority = 3,
//         .task_stack_size = 2048,
//         .task_core_id = 1
//     };

//     esp_event_loop_handle_t loop_handle;

//     esp_event_loop_create(&loop_args, &loop_handle);

//     esp_event_handler_register_with(loop_handle, "TEST", 1, run_on_event, data);
//     esp_event_handler_register_with(loop_handle, "TEST", 2, run_on_event, data2);   

//             esp_event_post_to(loop_handle, TEST, 1,data , sizeof(data),portMAX_DELAY);
//         vTaskDelay(1000/portTICK_RATE_MS);

//     // while (1)
//     // {

//         esp_event_post_to(loop_handle, TEST, 2, data2,sizeof(data2),portMAX_DELAY);
//         vTaskDelay(1000/portTICK_RATE_MS);
//         esp_event_handler_unregister_with(loop_handle, TEST, 1, run_on_event);
//         vTaskDelay(1000/portTICK_RATE_MS);
//         esp_event_post_to(loop_handle, TEST, 1,data , sizeof(data),portMAX_DELAY);
//         vTaskDelay(1000/portTICK_RATE_MS); 
//         esp_event_post_to(loop_handle, TEST, 2, data2,sizeof(data2),portMAX_DELAY);
//         vTaskDelay(1000/portTICK_RATE_MS); 
//         esp_event_handler_register_with(loop_handle, "TEST", 1, run_on_event, data);
//         esp_event_post_to(loop_handle, TEST, 1,data , sizeof(data),portMAX_DELAY);
//         vTaskDelay(1000/portTICK_RATE_MS);
//     // }

//     // esp_event_handler_unregister_with(loop_handle, TEST, 1, run_on_event);  we can call esp_event_post_to but loop so not anything
//     vTaskDelay(1000/portTICK_RATE_MS);
//     esp_event_loop_delete(loop_handle); // after use this function , we can not call esp_event_post_to againe
    
// }
