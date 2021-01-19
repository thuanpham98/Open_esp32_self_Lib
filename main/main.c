#include <stdio.h>
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

#define NO_DATA_TIMEOUT_SEC 10

static const char *TAG = "WEBSOCKET";

static TimerHandle_t shutdown_signal_timer;
static SemaphoreHandle_t shutdown_sema;

#define EXAMPLE_ESP_WIFI_SSID      "dara.vn"
#define EXAMPLE_ESP_WIFI_PASS      "6868686868"
#define EXAMPLE_ESP_MAXIMUM_RETRY  10
#define CONFIG_WEBSOCKET_URI       "wss://tnjqqhs9o8.execute-api.ap-southeast-1.amazonaws.com/test"

static EventGroupHandle_t s_wifi_event_group;
char * test = "POST /test HTTP/1.1 \
Host: wss://tnjqqhs9o8.execute-api.ap-southeast-1.amazonaws.com \
{\"action\":\"onMessage\",\"id\":\"esp32\"} \
Content-Type: application/json" ;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

typedef void (*state_get_wifi)(void* data);
typedef void (*state_reconnect)(void *data);
typedef void (*state_connected)(void *data);
typedef void(* react_event_connected)(void *data);
typedef void(* react_event_disconnected)(void *data);
typedef void(* react_event_data)(void *data);
typedef void(* react_event_error)(void *data);

void get_wifi(void *data){
    esp_wifi_connect();
}
void reconnect_wifi(void *data){
    if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
        esp_wifi_connect();
        s_retry_num++;
        ESP_LOGI(TAG, "retry to connect to the AP");
    } else {
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    ESP_LOGI(TAG,"connect to the AP fail");
}
void connected_wifi(void *data){
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
}
void ws_connected(void *data){
    ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
}
void ws_disconnected(void *data){
    ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
}
void ws_receive_data(void *data){
    esp_websocket_event_data_t *_data =(esp_websocket_event_data_t *) data;
    ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
    ESP_LOGI(TAG, "Received opcode=%d", _data->op_code);
    ESP_LOGW(TAG, "Received=%.*s", _data->data_len, (char *)_data->data_ptr);
    ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", _data->payload_len, _data->data_len, _data->payload_offset);
}
void ws_error(void *data){
    ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
}

void map_event_to_state_get_wifi(state_get_wifi wifi_get, void* data){
    wifi_get(NULL);
}
void map_event_to_state_reconnect(state_reconnect reconnect , void *data)
{
    reconnect(NULL);
}
void map_event_to_state_connected(state_connected connected , void *data){
    connected(data);
}
void ws_react_event_connected(react_event_connected connected,void* data){
    connected(NULL);
}
void ws_react_event_disconnected(react_event_disconnected disconnected,void *data){
    disconnected(NULL);
}
void ws_react_event_data(react_event_data receive_data,void *data){
    receive_data(data);
}
void ws_react_event_error(react_event_error error , void *data){
    error(NULL);
}

static void mapEventToState_Wifi(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        map_event_to_state_get_wifi(get_wifi,NULL);
    }else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        map_event_to_state_reconnect(reconnect_wifi,NULL);
    }else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        map_event_to_state_connected(connected_wifi,event_data);
    }
}

static void mapEventToState_Websocket(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ws_react_event_connected(ws_connected,NULL);
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ws_react_event_disconnected(ws_disconnected,NULL);
            break;
        case WEBSOCKET_EVENT_DATA:
            ws_react_event_data(ws_receive_data,data);
            // xTimerReset(shutdown_signal_timer, portMAX_DELAY);
            break;
        case WEBSOCKET_EVENT_ERROR:
            ws_react_event_error(ws_error,NULL);
            break;
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &mapEventToState_Wifi,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &mapEventToState_Wifi,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

static void shutdown_signaler(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG, "No data received for %d seconds, signaling shutdown", NO_DATA_TIMEOUT_SEC);
    xSemaphoreGive(shutdown_sema);
}

int n=0;

/* make json to post */
static char *Print_JSON()
{
    cJSON *sudo = cJSON_CreateObject();
    // cJSON *request = cJSON_CreateObject();
    // cJSON *body = cJSON_CreateObject();
;
    cJSON_AddItemToObject(sudo, "action", cJSON_CreateString("onMessage"));
    cJSON_AddItemToObject(sudo, "id", cJSON_CreateString("esp32"));


    char *a = cJSON_Print(sudo);
    cJSON_Delete(sudo); //if don't free, heap memory will be overload
    // ESP_LOGI(TAG,"%s",a);
    return a;
}

static void websocket_app_start(void)
{
    esp_websocket_client_config_t websocket_cfg = {};

    shutdown_signal_timer = xTimerCreate("Websocket shutdown timer", NO_DATA_TIMEOUT_SEC * 1000 / portTICK_PERIOD_MS,
                                         pdFALSE, NULL, shutdown_signaler);
    shutdown_sema = xSemaphoreCreateBinary();
    
    websocket_cfg.uri = CONFIG_WEBSOCKET_URI;
    websocket_cfg.subprotocol ="soap";
    websocket_cfg.transport=WEBSOCKET_TRANSPORT_OVER_SSL;


    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, mapEventToState_Websocket, (void *)client);

    esp_websocket_client_start(client);
    xTimerStart(shutdown_signal_timer, portMAX_DELAY);
    
    int i = 0;
    while (i < 10) {
        if(esp_websocket_client_is_connected(client)){
            char *data = Print_JSON();
            ESP_LOGI(TAG,"%s",data);
            esp_websocket_client_send_text(client, data, sizeof(data), portMAX_DELAY);
            free(data);

            while(esp_websocket_client_is_connected(client)) {
                    char data[] = "{\"action\":\"onMessage\",\"id\":\"esp32\"}";
                // Print_JSON();
                // "{\"action\":\"onMessage\",\"id\":\"esp32\"}";
                ESP_LOGI(TAG,"%s",data);

                    esp_websocket_client_send_text(client, data, sizeof(data), portMAX_DELAY);
                // free(data);

                vTaskDelay(1000/ portTICK_RATE_MS);
            }
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    xSemaphoreTake(shutdown_sema, portMAX_DELAY);
    esp_websocket_client_stop(client);
    ESP_LOGI(TAG, "Websocket Stopped");
    esp_websocket_client_destroy(client);
}


void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("WEBSOCKET_CLIENT", ESP_LOG_DEBUG);
    esp_log_level_set("TRANS_TCP", ESP_LOG_DEBUG);

    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    websocket_app_start();
}
