#ifndef _HID_BUTTON_H_
#define _HID_BUTTON_H_

typedef enum {
    BTN_NONE = 0,
    BTN_UP_PRESSED ,
    BTN_DOWN_PRESSED,
    BTN_PLAY_PRESSED,
    BTN_PLAY_LONG_PRESSED,
    BTN_NEXT_PRESSED,
    BTN_NEXT_LONG_PRESSED,
    BTN_PREV_PRESSED,
    BTN_PREV_LONG_PRESSED,
} hid_btn_event_t;

void esp_hid_button_init(void);
hid_btn_event_t esp_hid_button_readkey();
#endif
