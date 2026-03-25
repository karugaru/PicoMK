#include "hid.h"
#include "../keyboard/code.h"
#include "../settings/settings.h"

// clang-format off
const uint8_t hid_descriptor[] = {

    /** Keyboard */
    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x06, // Usage (Keyboard)
    0xA1, 0x01, // Collection (Application)

    0x85, KEYBOARD_REPORT_ID, // Report ID

    // Modifier byte

    0x75, 0x01, // Report Size (1)
    0x95, 0x08, // Report Count (8)
    0x05, 0x07, // Usage Page (Key codes)
    0x19, 0xE0, // Usage Minimum (Keyboard LeftControl)
    0x29, 0xE7, // Usage Maxium (Keyboard Right GUI)
    0x15, 0x00, // Logical Minimum (0)
    0x25, 0x01, // Logical Maximum (1)
    0x81, 0x02, // Input (Data, Variable, Absolute)

    // Reserved byte

    0x75, 0x01, // Report Size (1)
    0x95, 0x08, // Report Count (8)
    0x81, 0x03, // Input (Constant, Variable, Absolute)

    // LED report + padding

    0x95, 0x05, // Report Count (5)
    0x75, 0x01, // Report Size (1)
    0x05, 0x08, // Usage Page (LEDs)
    0x19, 0x01, // Usage Minimum (Num Lock)
    0x29, 0x05, // Usage Maxium (Kana)
    0x91, 0x02, // Output (Data, Variable, Absolute)

    0x95, 0x01, // Report Count (1)
    0x75, 0x03, // Report Size (3)
    0x91, 0x03, // Output (Constant, Variable, Absolute)

    // Keycodes

    0x95, 0x06, // Report Count (6)
    0x75, 0x08, // Report Size (8)
    0x15, 0x00, // Logical Minimum (0)
    0x25, 0xFF, // Logical Maximum (255)
    0x05, 0x07, // Usage Page (Key codes)
    0x19, 0x00, // Usage Minimum (Reserved (no event indicated))
    0x29, 0xFF, // Usage Maxium (Reserved)
    0x81, 0x00, // Input (Data, Array)

    0xC0, // End collection

    /** Mouse */
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x02, // USAGE (Mouse)
    0xA1, 0x01, // COLLECTION (Application)

    0x85, MOUSE_REPORT_ID, // Report ID

    0x09, 0x01, //   USAGE (Pointer)
    0xA1, 0x00, //   COLLECTION (Physical)

    // Buttons
    0x05, 0x09, //     USAGE_PAGE (Button)
    0x19, 0x01, //     USAGE_MINIMUM (Button 1)
    0x29, 0x03, //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00, //     LOGICAL_MINIMUM (0)
    0x25, 0x01, //     LOGICAL_MAXIMUM (1)
    0x95, 0x03, //     REPORT_COUNT (3)
    0x75, 0x01, //     REPORT_SIZE (1)
    0x81, 0x02, //     INPUT (Data,Var,Abs)
    0x95, 0x01, //     REPORT_COUNT (1)
    0x75, 0x05, //     REPORT_SIZE (5)
    0x81, 0x03, //     INPUT (Cnst,Var,Abs)

    // X, Y, W position deltas
    0x05, 0x01, //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30, //     USAGE (X)
    0x09, 0x31, //     USAGE (Y)
    0x09, 0x38, //     USAGE (Wheel)
    0x15, 0x81, //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F, //     LOGICAL_MAXIMUM (127)
    0x75, 0x08, //     REPORT_SIZE (8)
    0x95, 0x03, //     REPORT_COUNT (3)
    0x81, 0x06, //     INPUT (Data,Var,Rel)

    0xC0, //   END_COLLECTION
    0xC0, // END_COLLECTION

    /** Consumer Control */
    0x05, 0x0C, // Usage Page (Consumer)
    0x09, 0x01, // Usage (Consumer Control)
    0xA1, 0x01, // Collection (Application)

    0x85, CONSUMER_REPORT_ID, // Report ID

    0x15, 0x00,       // Logical Minimum (0)
    0x26, 0xFF, 0x03, // Logical Maximum (1023)
    0x19, 0x00,       // Usage Minimum (0)
    0x2A, 0xFF, 0x03, // Usage Maxium (1023)
    0x75, 0x10,       // Report Size (16)
    0x95, 0x06,       // Report Count (6)
    0x81, 0x00,       // Input (Data, Array)

    0xC0, // End Collection
};
// clang-format on

const uint8_t hid_descriptor_len = sizeof(hid_descriptor);

/**
 * @brief キーボードレポートをHID形式に変換する。
 * @param event 変換されるキーボードイベント
 * @param report 変換後のHIDレポート
 */
void hid_keyboard_to_report(hid_state_t *event,
                            uint8_t report[HID_KEYBOARD_REPORT_SIZE]) {
  // 修飾子、リザーブ、キーコード、同様に5つ分のキーコード
  report[0] = event->keyboard.real_modifier | event->keyboard.virtual_modifier;
  report[1] = 0;
  for (int i = 0; i < 6; i++) {
    report[2 + i] = event->keyboard.keycode[i];
  }
}

/**
 * @brief
 * マウスレポートをHID形式に変換し、変換した分のマウスイベントを消費する。
 * @param event 変換されるマウスイベント
 * @param report 変換後のHIDレポート
 */
void hid_mouse_to_report_and_consume(hid_state_t *event,
                                     uint8_t report[HID_MOUSE_REPORT_SIZE]) {
  int16_t x = 0;
  int16_t y = 0;
  int16_t w = 0;
  mouse_button_code_t buttons = 0;

  for (int i = 0; i < event->pointing_id_max; i++) {
    buttons |= event->mouse[i].buttons;

    int16_t x_step = event->mouse[i].xDelta / MOUSE_MOVE_THRESH;
    int16_t y_step = event->mouse[i].yDelta / MOUSE_MOVE_THRESH;
    int16_t w_step = event->mouse[i].wDelta / MOUSE_WHEEL_THRESH;

    x += x_step;
    y += y_step;
    w += w_step;

    event->mouse[i].xDelta -= x_step * MOUSE_MOVE_THRESH;
    event->mouse[i].yDelta -= y_step * MOUSE_MOVE_THRESH;
    event->mouse[i].wDelta -= w_step * MOUSE_WHEEL_THRESH;
  }

  // ボタン、X移動量、Y移動量、ホイール移動量
  report[0] = buttons;
  report[1] = (uint8_t)MAX(MIN(x, 127), -127);
  report[2] = (uint8_t)MAX(MIN(y, 127), -127);
  report[3] = (uint8_t)MAX(MIN(w, 127), -127);
}

/**
 * @brief コンシューマコントロールレポートをHID形式に変換する。
 * @param event 変換されるコンシューマコントロールイベント
 * @param report 変換後のHIDレポート
 */
void hid_consumer_to_report(hid_state_t *event,
                            uint8_t report[HID_CONSUMER_REPORT_SIZE]) {
  // キーコードの下位バイト、上位バイト、同様に5つ分のキーコード
  for (int i = 0; i < 6; i++) {
    consumer_code_t keycode = event->consumer.keycode[i];
    report[i * 2] = (uint8_t)(keycode & 0xFF);            // 下位バイト
    report[i * 2 + 1] = (uint8_t)((keycode >> 8) & 0xFF); // 上位バイト
  }
}

/**
 * @brief ポインティングデバイスIDを要求する。
 * @param state HID状態
 * @return 割り当てられたデバイスID、割り当てできなかった場合は-1
 */
int8_t hid_request_pointing_device_id(hid_state_t *state) {
  if (state->pointing_id_max >= HID_POINTING_DEVICE_MAX) {
    return -1;
  }

  state->mouse[state->pointing_id_max].buttons = 0;
  state->mouse[state->pointing_id_max].xDelta = 0;
  state->mouse[state->pointing_id_max].yDelta = 0;
  state->mouse[state->pointing_id_max].wDelta = 0;

  state->pointing_id_max++;
  return state->pointing_id_max - 1;
}
