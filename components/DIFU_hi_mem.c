#include "include/DIFU_hi_mem.h"

static esp_hi_mem_handler *internal_handler;

esp_err_t esp_hi_mem_malloc(esp_hi_mem_handler *handler){
    internal_handler = handler;
    esp_err_t ret = ESP_OK;
    ret = esp_himem_alloc(internal_handler->num_block*internal_handler->size_block*BLOCK_SIZE_BASE,internal_handler->addr_handler);
    if(!ret){
        return ret;
    }
    ret =esp_himem_alloc_map_range(internal_handler->size_block*BLOCK_SIZE_BASE, internal_handler->block_handler);
    if(!ret){
        return ret;
    }
    return ret;
}
esp_err_t esp_hi_meme_intergrate(esp_hi_mem_handler *handler,callback cp){
    internal_handler = handler;
    esp_err_t ret = ESP_OK;
    ret =esp_himem_map(*(internal_handler->addr_handler),
                        *(internal_handler->block_handler), 
                        internal_handler->addr_offset, 
                        0, 
                        32*1024, 
                        0, 
                        (void**)(&(internal_handler->pointer_handler))
    );/* take ram with every range block */
    if(!ret){
        return ret;
    }

    ret = cp(internal_handler->pointer_handler);

    ret =esp_himem_unmap(internal_handler->block_handler,
                        internal_handler->pointer_handler, 
                        internal_handler->size_block*BLOCK_SIZE_BASE);
    if(!ret){
        return ret;
    }
    return ret;
}
esp_err_t esp_hi_mem_free(esp_hi_mem_handler *handler){
    internal_handler = handler;
    esp_err_t ret = ESP_OK;
    ret = esp_himem_free(internal_handler->addr_handler);
    if(!ret){
        return ret;
    }
    ret = esp_himem_free_map_range(internal_handler->block_handler);
    if(!ret){
        return ret;
    }
    return ret;
}