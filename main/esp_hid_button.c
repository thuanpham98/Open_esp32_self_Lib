/* This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this software is
   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */

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
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

#include "esp_hidh.h"
#include "esp_hid_gap.h"
#include "include/esp_hid_button.h"

static const char *TAG = "HID_BUTTON";



static bool is_open = false;
static QueueHandle_t _btnEventQueue;

void hidh_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    esp_hidh_event_t event = (esp_hidh_event_t)id;
    esp_hidh_event_data_t *param = (esp_hidh_event_data_t *)event_data;

    switch (event) {
        case ESP_HIDH_OPEN_EVENT: {
            const uint8_t *bda = esp_hidh_dev_bda_get(param->open.dev);
            ESP_LOGI(TAG, ESP_BD_ADDR_STR " OPEN: %s", ESP_BD_ADDR_HEX(bda), esp_hidh_dev_name_get(param->open.dev));
            esp_hidh_dev_dump(param->open.dev, stdout);
            break;
        }
        case ESP_HIDH_BATTERY_EVENT: {
            const uint8_t *bda = esp_hidh_dev_bda_get(param->battery.dev);
            ESP_LOGI(TAG, ESP_BD_ADDR_STR " BATTERY: %d%%", ESP_BD_ADDR_HEX(bda), param->battery.level);
            break;
        }
        case ESP_HIDH_INPUT_EVENT: {
            const uint8_t *bda = esp_hidh_dev_bda_get(param->input.dev);
            ESP_LOGI(TAG, ESP_BD_ADDR_STR " INPUT: %8s, MAP: %2u, ID: %3u, Len: %d, Data:", ESP_BD_ADDR_HEX(bda), esp_hid_usage_str(param->input.usage), param->input.map_index, param->input.report_id, param->input.length);
            ESP_LOG_BUFFER_HEX(TAG, param->input.data, param->input.length);
            int key = BTN_NONE;
            if (param->input.data[0] == 0xCD) {
                key = BTN_PLAY_PRESSED;
            } else if (param->input.data[0] == 0xB5) {
                key = BTN_NEXT_PRESSED;
            } else if (param->input.data[0] == 0xB6) {
                key = BTN_PREV_PRESSED;
            } else if (param->input.data[0] == 0xE9) {
                key = BTN_UP_PRESSED;
            } else if (param->input.data[0] == 0xEA) {
                key = BTN_DOWN_PRESSED;
            } else if (param->input.data[0] == 0x23 && param->input.data[1] == 0x02) {
                key = BTN_PLAY_LONG_PRESSED;
            }else key = 20;

            if (key != BTN_NONE) {
                xQueueSend(_btnEventQueue, &key, portMAX_DELAY);
            }

            //Play, pause: cd 00, long: 23 02
            //next: b5 00, long: 40 00
            //prev: b6 00, long: 30 00
            //vol+: E9 00, long: 40 00 | 40 00
            //vol-: EA 00, long: 40 00 | 40 00
            break;
        }
        case ESP_HIDH_FEATURE_EVENT: {
            const uint8_t *bda = esp_hidh_dev_bda_get(param->feature.dev);
            ESP_LOGI(TAG, ESP_BD_ADDR_STR " FEATURE: %8s, MAP: %2u, ID: %3u, Len: %d", ESP_BD_ADDR_HEX(bda), esp_hid_usage_str(param->feature.usage), param->feature.map_index, param->feature.report_id, param->feature.length);
            ESP_LOG_BUFFER_HEX(TAG, param->feature.data, param->feature.length);
            break;
        }
        case ESP_HIDH_CLOSE_EVENT: {
            const uint8_t *bda = esp_hidh_dev_bda_get(param->close.dev);
            ESP_LOGI(TAG, ESP_BD_ADDR_STR " CLOSE: '%s' %s", ESP_BD_ADDR_HEX(bda), esp_hidh_dev_name_get(param->close.dev), esp_hid_disconnect_reason_str(esp_hidh_dev_transport_get(param->close.dev), param->close.reason));
            //MUST call this function to free all allocated memory by this device
            esp_hidh_dev_free(param->close.dev);
            is_open = false;
            break;
        }
        default:
            ESP_LOGI(TAG, "EVENT: %d", event);
            break;
    }
}

#define SCAN_DURATION_SECONDS 5

void hid_btn_task(void *pvParameters)
{
    size_t results_len = 0;
    esp_hid_scan_result_t *results = NULL;
    while (results_len == 0) {
        ESP_LOGI(TAG, "SCAN...");
        //start scan for HID devices
        esp_hid_scan(SCAN_DURATION_SECONDS, &results_len, &results);
        ESP_LOGI(TAG, "SCAN: %u results", results_len);
    }

    if (results_len) {
        esp_hid_scan_result_t *r = results;
        esp_hid_scan_result_t *cr = NULL;
        while (r) {
            printf("  %s: " ESP_BD_ADDR_STR ", ", (r->transport == ESP_HID_TRANSPORT_BLE) ? "BLE" : "BT ", ESP_BD_ADDR_HEX(r->bda));
            printf("RSSI: %d, ", r->rssi);
            printf("USAGE: %s, ", esp_hid_usage_str(r->usage));
            if (r->transport == ESP_HID_TRANSPORT_BLE) {
                cr = r;
                printf("APPEARANCE: 0x%04x, ", r->ble.appearance);
                printf("ADDR_TYPE: '%s', ", ble_addr_type_str(r->ble.addr_type));
            } else {
                cr = r;
                printf("COD: %s[", esp_hid_cod_major_str(r->bt.cod.major));
                esp_hid_cod_minor_print(r->bt.cod.minor, stdout);
                printf("] srv 0x%03x, ", r->bt.cod.service);
                print_uuid(&r->bt.uuid);
                printf(", ");
            }
            printf("NAME: %s ", r->name ? r->name : "");
            printf("\n");
            r = r->next;
        }

        while (1) {
            if (!is_open) {
                ESP_LOGI(TAG, "Opening...");
                if (esp_hidh_dev_open(cr->bda, cr->transport, cr->ble.addr_type) != NULL) {
                    is_open = true;
                }

            }
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        //free the results
        esp_hid_scan_results_free(results);
    }
    vTaskDelete(NULL);
}

void esp_hid_button_init(void)
{
    esp_err_t ret;
    ESP_ERROR_CHECK(esp_hid_gap_init(ESP_BT_MODE_BTDM));
    ESP_ERROR_CHECK(esp_ble_gattc_register_callback(esp_hidh_gattc_event_handler));
    esp_hidh_config_t config = {
        .callback = hidh_callback,
    };
    ESP_ERROR_CHECK(esp_hidh_init(&config));

    _btnEventQueue = xQueueCreate(10, sizeof(int));

    xTaskCreate(&hid_btn_task, "hid_task",  6* 1024, NULL, 5, NULL);
}

hid_btn_event_t esp_hid_button_readkey()
{
    int key = -1;
    xQueueReceive(_btnEventQueue, &key, portMAX_DELAY);
    return key;
}
