#ifndef __DIFU_HI_MEM_H
#define __DIFU_HI_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_heap_caps.h"
#include "esp32/spiram.h"
#include "esp32/himem.h"

#define BLOCK_SIZE_BASE 32*1024

typedef struct esp_hi_mem_handler 
{
    esp_himem_handle_t *addr_handler;
    esp_himem_rangehandle_t *block_handler;
    void *pointer_handler;
    size_t num_block;
    size_t size_block;
    size_t addr_offset;
    size_t block_offset;

} esp_hi_mem_handler;

typedef esp_err_t (*callback)(void* data);

esp_err_t esp_hi_mem_malloc(esp_hi_mem_handler *handler);
esp_err_t esp_hi_meme_intergrate(esp_hi_mem_handler *handler,callback cp);
esp_err_t esp_hi_mem_free(esp_hi_mem_handler *handler);

#ifdef __cplusplus
}
#endif

#endif