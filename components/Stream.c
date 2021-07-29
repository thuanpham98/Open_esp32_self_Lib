#include "Stream.h"


esp_err_t stream_init(stream_t stream){

    esp_err_t ret;
    char _event_stream_custom[32];
    memcpy(_event_stream_custom,stream.event_stream,32);

    ESP_EVENT_DECLARE_BASE(_event_stream_custom);

    ESP_EVENT_DEFINE_BASE(_event_stream_custom);

    ret = esp_event_loop_create(&(stream.stream_agr), &(stream.stream_handler));

    if(ret!= ESP_FAIL){
        return esp_event_handler_register_with(stream.stream_handler, stream.event, stream.thread, stream.onData, stream.data_event);
    }
    return ret;
}
esp_err_t stream_add_event(stream_t stream){
    return esp_event_post_to(stream.stream_handler, stream.event, stream.thread,stream.data_event , sizeof(stream.data_event),stream.time_out);

}
esp_err_t stream_pause(stream_t stream){
    return esp_event_handler_unregister_with(stream.stream_handler, stream.event, stream.thread, stream.onData);
}
esp_err_t stream_resumse(stream_t stream){
    return esp_event_handler_register_with(stream.stream_handler, stream.event, stream.thread, stream.onData, stream.data_event);
}
esp_err_t stream_cancel(stream_t stream){
    return esp_event_loop_delete(stream.stream_handler);
}