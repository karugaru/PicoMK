#ifndef PICOMK_HID_H
#define PICOMK_HID_H

#include "../keyboard/code.h"
#include <pico/stdlib.h>

#define KEYBOARD_REPORT_ID 0x01
#define MOUSE_REPORT_ID 0x02
#define CONSUMER_REPORT_ID 0x03
#define REPORT_ID_MAX CONSUMER_REPORT_ID

#define HID_KEYBOARD_REPORT_SIZE 8
#define HID_MOUSE_REPORT_SIZE 4
#define HID_CONSUMER_REPORT_SIZE 12
#define HID_REPORT_SIZE_MAX                                                    \
  (MAX(MAX(HID_CONSUMER_REPORT_SIZE, HID_KEYBOARD_REPORT_SIZE),                \
       HID_MOUSE_REPORT_SIZE))

#define HID_POINTING_DEVICE_MAX 2

extern const uint8_t hid_descriptor[];
extern const uint8_t hid_descriptor_len;

// HID内部状態
typedef struct {
  bool has_keyboard_event;
  struct {
    keyboard_modifier_bits_t real_modifier;
    keyboard_modifier_bits_t virtual_modifier;
    keyboard_code_t keycode[6];
  } keyboard;

  bool has_mouse_event;
  struct {
    mouse_button_code_t buttons;
    int16_t xDelta;
    int16_t yDelta;
    int16_t wDelta;
  } mouse[HID_POINTING_DEVICE_MAX];
  int8_t pointing_id_max;

  bool has_consumer_event;
  struct {
    consumer_code_t keycode[6];
  } consumer;
} hid_state_t;

void hid_keyboard_to_report(hid_state_t *event,
                            uint8_t report[HID_KEYBOARD_REPORT_SIZE]);
void hid_mouse_to_report_and_consume(hid_state_t *event,
                                     uint8_t report[HID_MOUSE_REPORT_SIZE]);
void hid_consumer_to_report(hid_state_t *event,
                            uint8_t report[HID_CONSUMER_REPORT_SIZE]);

int8_t hid_request_pointing_device_id(hid_state_t *state);

#endif // PICOMK_HID_H
