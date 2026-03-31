#include <pico/bootrom.h>
#include <stdio.h>
#include <stdlib.h>

#include "../hid/hid.h"
#include "../settings/keymap.h"
#include "../settings/settings.h"
#include "../state/state.h"
#include "event.h"

#ifndef DEBUG_EVENT
#define DEBUG_EVENT 0
#endif

static hid_state_t hid_state = {0};
static int8_t pointing_device_mouse_key = -1;
static bool mouse_move_key_pressed[6] = {false};

typedef enum {
  MOUSE_MOVE_UP = 0,
  MOUSE_MOVE_DOWN,
  MOUSE_MOVE_LEFT,
  MOUSE_MOVE_RIGHT,
  MOUSE_WHEEL_UP,
  MOUSE_WHEEL_DOWN,
} mouse_move_key_index_t;

static int8_t mouse_move_icode_to_index(icode_t icode) {
  switch (icode) {
  case IMC_MOUSE_MOVE_UP:
    return MOUSE_MOVE_UP;
  case IMC_MOUSE_MOVE_DOWN:
    return MOUSE_MOVE_DOWN;
  case IMC_MOUSE_MOVE_LEFT:
    return MOUSE_MOVE_LEFT;
  case IMC_MOUSE_MOVE_RIGHT:
    return MOUSE_MOVE_RIGHT;
  case IMC_MOUSE_WHEEL_UP:
    return MOUSE_WHEEL_UP;
  case IMC_MOUSE_WHEEL_DOWN:
    return MOUSE_WHEEL_DOWN;
  default:
    return -1;
  }
}

static bool reached_threshold(int16_t value, int16_t threshold) {
  return value >= threshold || value <= -threshold;
}

/**
 * @brief マウス移動イベントがあるかを取得する。
 * @return マウス移動イベントがある場合はtrueを返す
 */
static bool event_has_mouse_move_event() {
  for (int i = 0; i < hid_state.pointing_id_max; i++) {
    if (reached_threshold(hid_state.mouse[i].xDelta, MOUSE_MOVE_THRESH) ||
        reached_threshold(hid_state.mouse[i].yDelta, MOUSE_MOVE_THRESH) ||
        reached_threshold(hid_state.mouse[i].wDelta, MOUSE_WHEEL_THRESH)) {
      return true;
    }
  }

  return false;
}

/**
 * @brief イベント処理の初期化
 */
void event_init(void) {
  int8_t id = hid_request_pointing_device_id(&hid_state);
  if (id >= 0) {
    pointing_device_mouse_key = id;
  }
}

/**
 * @brief ポインティングデバイスIDの要求
 * @return 割り当てられたポインティングデバイスID、割り当てできなかった場合は-1
 */
int8_t event_request_pointing_device_id(void) {
  return hid_request_pointing_device_id(&hid_state);
}

/**
 * @brief キーボードキーを追加
 */
bool event_apply_press_keyboard_key(keyboard_modifiered_code_t keycode) {
  // 修飾キーが直接指定された場合
  if (keycode >= ICODE_MODIFIER_START && keycode <= ICODE_MODIFIER_END) {
    keyboard_modifier_bits_t old_real_mod = hid_state.keyboard.real_modifier;
    keyboard_modifier_bits_t new_real_mod =
        old_real_mod | code_icode_to_modifier((icode_t)keycode);
    keyboard_modifier_bits_t virt_mod = hid_state.keyboard.virtual_modifier;

    hid_state.keyboard.real_modifier = new_real_mod;
    return (old_real_mod | virt_mod) != (new_real_mod | virt_mod);
  }

  // コードから修飾子ビットとキーコードを抽出
  keyboard_modifier_bits_t mod_bits = code_icode_extract_modifier_bits(keycode);
  keyboard_modifier_bits_t key_bits = 0xFF & keycode;
  keyboard_modifier_bits_t old_virt_mod = hid_state.keyboard.virtual_modifier;
  hid_state.keyboard.virtual_modifier |= mod_bits;

  bool virt_mod_changed = old_virt_mod != hid_state.keyboard.virtual_modifier;

  // 既に押されているかチェック
  for (int i = 0; i < 6; i++) {
    if (hid_state.keyboard.keycode[i] == key_bits) {
      return false || virt_mod_changed;
    }
  }

  // 空きスロットを探して追加
  for (int i = 0; i < 6; i++) {
    if (hid_state.keyboard.keycode[i] == 0) {
      hid_state.keyboard.keycode[i] = key_bits;
      return true;
    }
  }

  return false || virt_mod_changed; // スロットが満杯
}

/**
 * @brief キーボードキーを削除
 */
bool event_apply_release_keyboard_key(keyboard_modifiered_code_t keycode) {
  // 修飾キーが直接指定された場合
  if (keycode >= ICODE_MODIFIER_START && keycode <= ICODE_MODIFIER_END) {
    keyboard_modifier_bits_t old_real_mod = hid_state.keyboard.real_modifier;
    keyboard_modifier_bits_t new_real_mod =
        old_real_mod & ~code_icode_to_modifier((icode_t)keycode);
    keyboard_modifier_bits_t virt_mod = hid_state.keyboard.virtual_modifier;

    hid_state.keyboard.real_modifier = new_real_mod;
    return (old_real_mod | virt_mod) != (new_real_mod | virt_mod);
  }

  // コードから修飾子ビットとキーコードを抽出
  keyboard_modifier_bits_t mod_bits = code_icode_extract_modifier_bits(keycode);
  keyboard_modifier_bits_t key_bits = 0xFF & keycode;
  keyboard_modifier_bits_t old_virt_mod = hid_state.keyboard.virtual_modifier;
  hid_state.keyboard.virtual_modifier &= ~mod_bits;

  bool virt_mod_changed = old_virt_mod != hid_state.keyboard.virtual_modifier;

  for (int i = 0; i < 6; i++) {
    if (hid_state.keyboard.keycode[i] == key_bits) {
      // 見つかったキーを削除し、後ろのキーを前に詰める
      for (int j = i; j < 5; j++) {
        hid_state.keyboard.keycode[j] = hid_state.keyboard.keycode[j + 1];
      }
      hid_state.keyboard.keycode[5] = IKC_NOOP;
      return true;
    }
  }
  return false || virt_mod_changed; // キーが見つからない
}

/**
 * @brief コンシューマーキーを追加
 */
bool event_apply_press_consumer_key(consumer_code_t keycode) {
  // 既に押されているかチェック
  for (int i = 0; i < 6; i++) {
    if (hid_state.consumer.keycode[i] == keycode) {
      return false;
    }
  }

  // 空きスロットを探して追加
  for (int i = 0; i < 6; i++) {
    if (hid_state.consumer.keycode[i] == 0) {
      hid_state.consumer.keycode[i] = keycode;
      return true;
    }
  }

  return false; // スロットが満杯
}

/**
 * @brief コンシューマーキーを削除
 */
bool event_apply_release_consumer_key(consumer_code_t keycode) {
  for (int i = 0; i < 6; i++) {
    if (hid_state.consumer.keycode[i] == keycode) {
      // 見つかったキーを削除し、後ろのキーを前に詰める
      for (int j = i; j < 5; j++) {
        hid_state.consumer.keycode[j] = hid_state.consumer.keycode[j + 1];
      }
      hid_state.consumer.keycode[5] = 0;
      return true;
    }
  }
  return false; // キーが見つからない
}

/**
 * @brief 指定されたポインティングデバイスIDのマウス状態に移動量を加算する。
 * @param device_id ポインティングデバイスID
 * @param buttons マウスボタンの状態(最新値で更新)
 * @param x X軸の移動量
 * @param y Y軸の移動量
 * @param w ホイールの移動量
 */
void event_accumulate_mouse(uint8_t device_id, mouse_button_code_t buttons,
                            int8_t x, int8_t y, int8_t w) {
  if (device_id >= hid_state.pointing_id_max) {
    return;
  }

  mouse_button_code_t old_buttons = hid_state.mouse[device_id].buttons;

  hid_state.mouse[device_id].buttons = buttons;
  hid_state.mouse[device_id].xDelta += x;
  hid_state.mouse[device_id].yDelta += y;
  hid_state.mouse[device_id].wDelta += w;

  hid_state.has_mouse_event |= (old_buttons != buttons);
}

/**
 * @brief 標準的なイベントを処理する。
 *        内部コードから、標準キーコード、コンシューマコード、
 *        マウスコード、特殊コード
 *        などを判別・処理し、内部HID状態を更新する。
 * @param icode 内部コード
 * @param pressed 押された(true)か離された(false)か
 */
void event_process_standard(icode_t icode, bool pressed) {

  // ISC_BOOTが押されたらブートモードでリセット
  if (icode == ISC_BOOT && pressed) {
    state_set_system(STATE_BOOTLOADER);
    return;
  }

  // 接続モード切替コードの処理
  if (pressed) {
    connection_preference_t new_pref;
    bool handled = true;

    switch (icode) {
    case ISC_CONN_TOGGLE:
      new_pref = (state_get_connection_preference() == CONN_PREF_USB)
                     ? CONN_PREF_BLE
                     : CONN_PREF_USB;
      break;
    case ISC_CONN_USB:
      new_pref = CONN_PREF_USB;
      break;
    case ISC_CONN_BLE:
      new_pref = CONN_PREF_BLE;
      break;
    default:
      handled = false;
      break;
    }

    if (handled) {
      state_switch_connection(new_pref);
      return;
    }
  }

  // 標準キーコードの場合
  if (icode >= ICODE_STANDARD_START && icode <= ICODE_STANDARD_END) {
    keyboard_modifiered_code_t keycode = (keyboard_modifiered_code_t)icode;
    bool state_changed = false;
    if (pressed) {
      state_changed = event_apply_press_keyboard_key(keycode);
    } else {
      state_changed = event_apply_release_keyboard_key(keycode);
    }

    if (state_changed) {
      hid_state.has_keyboard_event = true;
    }
    return;
  }

  // コンシューマキーコードの場合
  if (icode >= ICODE_CONSUMER_START && icode <= ICODE_CONSUMER_END) {
    consumer_code_t keycode = code_icodes_to_consumer(icode);

    bool state_changed = false;
    if (pressed) {
      state_changed = event_apply_press_consumer_key(keycode);
    } else {
      state_changed = event_apply_release_consumer_key(keycode);
    }

    if (state_changed) {
      hid_state.has_consumer_event = true;
    }
    return;
  }

  // マウスボタンキーコードの場合
  if (icode >= ICODE_MOUSE_BUTTON_START && icode <= ICODE_MOUSE_BUTTON_END) {
    if (pointing_device_mouse_key < 0) {
      // マウスキーが割り当てられていない
      return;
    }
    mouse_button_code_t state_button =
        hid_state.mouse[pointing_device_mouse_key].buttons;

    mouse_button_code_t mouse_button = code_icodes_to_mouse_button(icode);
    if (pressed) {
      state_button |= mouse_button;
    } else {
      state_button &= ~mouse_button;
    }

    event_accumulate_mouse(pointing_device_mouse_key, state_button, 0, 0, 0);
    return;
  }

  // マウス移動キーコードの場合
  if (icode >= ICODE_MOUSE_MOVE_START && icode <= ICODE_MOUSE_MOVE_END) {
    if (pointing_device_mouse_key < 0) {
      return;
    }

    int8_t index = mouse_move_icode_to_index(icode);
    if (index >= 0) {
      mouse_move_key_pressed[index] = pressed;
    }
    return;
  }
}

/**
 * @brief 定期的なイベント処理を行う。
 */
void event_process_periodic(void) {
  if (pointing_device_mouse_key >= 0) {
    int8_t dx = 0;
    int8_t dy = 0;
    int8_t dw = 0;

    if (mouse_move_key_pressed[MOUSE_MOVE_UP]) {
      dy -= MOUSE_MOVE_DELTA;
    }
    if (mouse_move_key_pressed[MOUSE_MOVE_DOWN]) {
      dy += MOUSE_MOVE_DELTA;
    }
    if (mouse_move_key_pressed[MOUSE_MOVE_LEFT]) {
      dx -= MOUSE_MOVE_DELTA;
    }
    if (mouse_move_key_pressed[MOUSE_MOVE_RIGHT]) {
      dx += MOUSE_MOVE_DELTA;
    }
    if (mouse_move_key_pressed[MOUSE_WHEEL_UP]) {
      dw += MOUSE_WHEEL_DELTA;
    }
    if (mouse_move_key_pressed[MOUSE_WHEEL_DOWN]) {
      dw -= MOUSE_WHEEL_DELTA;
    }

    if (dx != 0 || dy != 0 || dw != 0) {
      mouse_button_code_t buttons =
          hid_state.mouse[pointing_device_mouse_key].buttons;
      event_accumulate_mouse(pointing_device_mouse_key, buttons, dx, dy, dw);
    }
  }

  if (event_has_mouse_move_event()) {
    hid_state.has_mouse_event = true;
  }
}

/**
 * @brief 内部HID状態から、発生すべきイベントがあるかを取得する。
 * @return イベントがある場合はtrueを返す
 */
bool event_has_event(void) {
  return hid_state.has_keyboard_event || hid_state.has_consumer_event ||
         hid_state.has_mouse_event || event_has_mouse_move_event();
}
/**
 * @brief
 * キーボードの内部状態が変化されているならば、HIDレポートとして取り出す。
 * @return レポートが取り出された場合にtrueを返す
 */
bool event_pop_keyboard_report(uint8_t report[HID_KEYBOARD_REPORT_SIZE]) {
  if (!hid_state.has_keyboard_event) {
    return false;
  }
  hid_keyboard_to_report(&hid_state, report);
  hid_state.has_keyboard_event = false;

#if DEBUG_EVENT
  // デバッグ出力
  printf("Keyboard Report: ");
  for (int i = 0; i < HID_KEYBOARD_REPORT_SIZE; i++) {
    printf("0x%02X ", report[i]);
  }
  printf("\n");
#endif

  return true;
}
/**
 * @brief
 * コンシューマの内部状態が変化されているならば、HIDレポートとして取り出す。
 * @return レポートが取り出された場合にtrueを返す
 */
bool event_pop_consumer_report(uint8_t report[HID_CONSUMER_REPORT_SIZE]) {
  if (!hid_state.has_consumer_event) {
    return false;
  }
  hid_consumer_to_report(&hid_state, report);
  hid_state.has_consumer_event = false;

#if DEBUG_EVENT
  // デバッグ出力
  printf("Consumer Report: ");
  for (int i = 0; i < HID_CONSUMER_REPORT_SIZE; i++) {
    printf("0x%02X ", report[i]);
  }
  printf("\n");
#endif

  return true;
}
/**
 * @brief マウスの内部状態が変化されているならば、HIDレポートとして取り出す。
 * @return レポートが取り出された場合にtrueを返す
 */
bool event_pop_mouse_report(uint8_t report[HID_MOUSE_REPORT_SIZE]) {
  if (!hid_state.has_mouse_event && !event_has_mouse_move_event()) {
    return false;
  }
  hid_mouse_to_report_and_consume(&hid_state, report);
  hid_state.has_mouse_event = false;

#if DEBUG_EVENT
  // デバッグ出力
  printf("Mouse Report: ");
  for (int i = 0; i < HID_MOUSE_REPORT_SIZE; i++) {
    printf("0x%02X ", report[i]);
  }
  printf("\n");
#endif

  return true;
}
