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
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"

typedef enum stream_type_t {
    STREAM_NONE = 0,
    STREAM_CONTROLLER,
    STREAM_SUPCRIPTION,
} stream_type_t;

typedef struct stream_config_t
{
    char *event_name;//
    uint8_t *data_event;//
    uint16_t *thread;//
    uint16_t num_thead;//
    esp_event_loop_handle_t loop_handle;//
    esp_event_handler_t onData;
    TickType_t time_out;//
    esp_event_loop_args_t loop_args;//

} stream_config_t;

typedef struct stream_config_t* g_stream_config_handler_t;
typedef struct stream_handler_t* g_stream_handler_t; 


g_stream_handler_t stream_init(g_stream_config_handler_t stream);
esp_err_t stream_add_event(g_stream_handler_t handler , uint16_t thread, size_t data_size);
esp_err_t stream_pause(g_stream_handler_t handler);
esp_err_t stream_resumse(g_stream_handler_t handler);
esp_err_t stream_cancel(g_stream_handler_t handler);

#ifdef __cplusplus
}
#endif

#endif