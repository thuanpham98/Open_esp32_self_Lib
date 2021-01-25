#include <stdio.h>
#include <stdlib.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/* library for Json */
#include <limits.h>
#include <ctype.h>
#include <cJSON.h>
#include <cJSON_Utils.h>
#include "unity.h"

#include "esp_heap_caps.h"
#include "esp32/spiram.h"
// #include "sdkconfig.h"
#include "esp32/himem.h"
#include "DIFU_hi_mem.h"

static esp_himem_handle_t _addr_handler;
static esp_himem_rangehandle_t _block_handler;

static const char *TAG = "OTA";
esp_err_t my_call_back(void *data){
    uint32_t *ptr  = (uint32_t *)data;
    *ptr = 100;
    *(ptr+20000)=1; 
    return ESP_OK;
}

esp_err_t my_call_back_read(void *data){
    uint32_t *ptr  = (uint32_t *)data;

    ESP_LOGI(TAG,"%d",*ptr);
    ESP_LOGI(TAG,"%d",*(ptr+20000)); 

    return ESP_OK;
}

void check_ram(size_t check_size){

    esp_hi_mem_handler my_handler;
    my_handler.addr_handler = &_addr_handler;
    my_handler.block_handler = &_block_handler;
    my_handler.addr_offset=0;
    my_handler.block_offset=0;
    my_handler.num_block=check_size/(32*1024);
    my_handler.size_block = 1;


    // init memory
    ESP_ERROR_CHECK(esp_hi_mem_malloc(&my_handler));

    ESP_LOGI(TAG,"init ok, start writing ");

    // loading data to memory
    for(size_t i =0 ; i < check_size ; i+= my_handler.size_block*BLOCK_SIZE_BASE){
        ESP_LOGI(TAG,"%d",i);
        my_handler.addr_offset=i;
        ESP_LOGI(TAG,"%d",my_handler.addr_offset);
        ESP_LOGI(TAG,"%d",my_handler.num_block);
        ESP_LOGI(TAG,"%d",my_handler.size_block*BLOCK_SIZE_BASE);
        // ESP_ERROR_CHECK(esp_hi_meme_intergrate(&my_handler,my_call_back));
        ESP_ERROR_CHECK(
            esp_himem_map(
                *(my_handler.addr_handler),
                *(my_handler.block_handler), 
                i, 
                0, 
                my_handler.size_block*BLOCK_SIZE_BASE, 
                0, 
                (void**)(&(my_handler.pointer_handler))
            )
        );
    }
    //  check memory again
    ESP_LOGI(TAG,"LOADING Again");
    for(size_t i =0 ; i < check_size ; i+=my_handler.size_block*BLOCK_SIZE_BASE){
        my_handler.addr_offset=i;
        ESP_LOGI(TAG,"%d",my_handler.addr_offset);
        ESP_ERROR_CHECK(esp_hi_meme_intergrate(&my_handler,my_call_back_read));
    }
    // free memory
    ESP_ERROR_CHECK(esp_hi_mem_free(&my_handler));
}

void app_main(void)
{

    size_t memcnt=esp_himem_get_phys_size();
    size_t memfree=esp_himem_get_free_size();
    printf("Himem has %dKiB of memory, %dKiB of which is free. Testing the free memory...\n", (int)memcnt/1024, (int)memfree/1024);
    check_ram(10*32*1024);
    printf("Done!\n");
    
}
