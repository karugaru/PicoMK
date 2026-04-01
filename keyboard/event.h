#ifndef PICOMK_EVENT_H
#define PICOMK_EVENT_H

#include "code.h"
#include <../hid/hid.h>

void event_init(void);
int8_t event_request_pointing_device_id(void);

bool event_apply_press_keyboard_key(keyboard_modifiered_code_t keycode);
bool event_apply_release_keyboard_key(keyboard_modifiered_code_t keycode);
bool event_apply_press_consumer_key(consumer_code_t keycode);
bool event_apply_release_consumer_key(consumer_code_t keycode);
void event_accumulate_mouse(uint8_t device_id, mouse_button_code_t buttons,
                            int8_t x, int8_t y, int8_t w);

void event_process_standard(icode_t icode, bool pressed);
void event_process_periodic(void);
bool event_has_event(void);

bool event_pop_keyboard_report(uint8_t report[HID_KEYBOARD_REPORT_SIZE]);
bool event_pop_consumer_report(uint8_t report[HID_CONSUMER_REPORT_SIZE]);
bool event_pop_mouse_report(uint8_t report[HID_MOUSE_REPORT_SIZE]);

#endif // PICOMK_EVENT_H
