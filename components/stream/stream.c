#include "stream.h"

static const char* TAG="STREAM";

typedef struct stream_handler_t
{
    g_stream_config_handler_t config;
    SemaphoreHandle_t         lock;
    // Semaphore
}stream_handler_t;

stream_handler_t * s_handler_t =NULL;

g_stream_handler_t stream_init(g_stream_config_handler_t stream){
    stream_handler_t *handler = (stream_handler_t *)malloc(sizeof(stream_handler_t));
    handler->lock =xSemaphoreCreateMutex();
    handler->config = stream;

    esp_event_loop_create(&(handler->config->loop_args), &(handler->config->loop_handle));
    for (size_t i = 0; i < handler->config->num_thead; i++)
    {
        esp_event_handler_register_with(handler->config->loop_handle, handler->config->event_name, *(handler->config->thread + i), handler->config->onData, handler->config->data_event);
    }
    
    s_handler_t = handler;
    return handler;
}

esp_err_t stream_add_event(g_stream_handler_t handler , uint16_t thread, size_t data_size){
    s_handler_t= handler;
    return esp_event_post_to(s_handler_t->config->loop_handle, s_handler_t->config->event_name, *(s_handler_t->config->thread+thread),(void *)s_handler_t->config->data_event , data_size,s_handler_t->config->time_out);
}

esp_err_t stream_pause(g_stream_handler_t handler){return 0;}

esp_err_t stream_resumse(g_stream_handler_t handler){return 0;}

esp_err_t stream_cancel(g_stream_handler_t handler){return 0;}


// esp_err_t stream_init(stream_t stream){

//     esp_err_t ret;
//     char _event_stream_custom[32];
//     memcpy(_event_stream_custom,stream.event_stream,32);

//     ESP_EVENT_DECLARE_BASE(_event_stream_custom);

//     ESP_EVENT_DEFINE_BASE(_event_stream_custom);

//     ret = esp_event_loop_create(&(stream.stream_agr), &(stream.stream_handler));

//     if(ret!= ESP_FAIL){
//         return esp_event_handler_register_with(stream.stream_handler, stream.event, stream.thread, stream.onData, stream.data_event);
//     }
//     return ret;
// }
// esp_err_t stream_add_event(stream_t stream){
//     return esp_event_post_to(stream.stream_handler, stream.event, stream.thread,stream.data_event , sizeof(stream.data_event),stream.time_out);

// }
// esp_err_t stream_pause(stream_t stream){
//     return esp_event_handler_unregister_with(stream.stream_handler, stream.event, stream.thread, stream.onData);
// }
// esp_err_t stream_resumse(stream_t stream){
//     return esp_event_handler_register_with(stream.stream_handler, stream.event, stream.thread, stream.onData, stream.data_event);
// }
// esp_err_t stream_cancel(stream_t stream){
//     return esp_event_loop_delete(stream.stream_handler);
// }