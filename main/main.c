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

#define BLOCK_SIZE 32*1024

static const char *TAG = "OTA";

void check_ram(size_t check_size){
    esp_himem_handle_t addr_handler;
    esp_himem_rangehandle_t actual_handler;

    // init memory
    ESP_ERROR_CHECK(esp_himem_alloc(check_size,&addr_handler));
    ESP_ERROR_CHECK(esp_himem_alloc_map_range(BLOCK_SIZE,&actual_handler)); // muilty of 32KB

    ESP_LOGI(TAG,"init ok, start writing ");
    // loading data to memory
    for(size_t i =0 ; i < check_size ; i+=BLOCK_SIZE){
        ESP_LOGI(TAG,"%d",i);
        uint32_t *ptr = NULL;

        ESP_ERROR_CHECK(esp_himem_map(addr_handler, actual_handler, i, 0, BLOCK_SIZE, 0, (void**)&ptr));/* take ram with every range block */
        *ptr = i;
        *(ptr+20000)=1; 
        ESP_ERROR_CHECK(esp_himem_unmap(actual_handler, ptr, BLOCK_SIZE));
    }
    //  check memory again
    ESP_LOGI(TAG,"LOADING Again");
    for(size_t i =0 ; i < check_size ; i+=BLOCK_SIZE){
        uint32_t *ptr ;
        ESP_ERROR_CHECK(esp_himem_map(addr_handler, actual_handler, i, 0, BLOCK_SIZE, 0, (void**)&ptr));/* take ram with every range block */
        ESP_LOGI(TAG,"%d",*ptr);
        ESP_LOGI(TAG,"%d",*(ptr+20000));
        ESP_ERROR_CHECK(esp_himem_unmap(actual_handler, ptr, BLOCK_SIZE));
    }
    ESP_LOGI(TAG,"---------------");
    ESP_LOGI(TAG,"%d",esp_himem_get_free_size());
    ESP_LOGI(TAG,"%d",esp_get_free_heap_size());
    // uint8_t *test = (uint8_t*) malloc( (esp_get_free_heap_size()-100000) * sizeof( uint8_t ));;
    // char st[100]= "fhieo";
    // test = (uint8_t *)st;
    ESP_LOGI(TAG,"---------------");
    ESP_LOGI(TAG,"%d",esp_himem_get_free_size());
    ESP_LOGI(TAG,"%d",esp_get_free_heap_size());
    // free memory
    ESP_ERROR_CHECK(esp_himem_free(addr_handler));
    ESP_ERROR_CHECK(esp_himem_free_map_range(actual_handler));
    // free(test);
    ESP_LOGI(TAG,"---------------");
    ESP_LOGI(TAG,"%d",esp_himem_get_free_size());
    ESP_LOGI(TAG,"%d",esp_get_free_heap_size());
}

void app_main(void)
{

    size_t memcnt=esp_himem_get_phys_size();
    size_t memfree=esp_himem_get_free_size();
    printf("Himem has %dKiB of memory, %dKiB of which is free. Testing the free memory...\n", (int)memcnt/1024, (int)memfree/1024);
    check_ram(10*BLOCK_SIZE);
    printf("Done!\n");
    
}
