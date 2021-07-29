#ifndef __STREAM_H
#define __STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

typedef enum {
    Stream_none = 0,
    Stream_Controller,
    Stream_Subcription,
} stream_type_t;

typedef struct 
{
    char event_stream[32];
    uint8_t data_event;
    uint16_t thread;
    esp_event_loop_handle_t stream_handler;
    esp_event_handler_t onData;
    TickType_t time_out;
    esp_event_loop_args_t stream_agr;

} stream_t;

esp_err_t stream_init(stream_t stream);
esp_err_t stream_add_event(stream_t stream);
esp_err_t stream_pause(stream_t stream);
esp_err_t stream_resumse(stream_t stream);
esp_err_t stream_cancel(stream_t stream);

#ifdef __cplusplus
}
#endif

#endif