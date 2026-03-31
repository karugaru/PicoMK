#include <hardware/watchdog.h>
#include <pico/bootrom.h>

#include "../ble/ble.h"
#include "../led/led.h"
#include "../usb/usb_hid.h"
#include "state.h"

typedef struct {
  int8_t required_from; // 遷移元の状態。-1の場合はどの状態からでも遷移可能。
  uint8_t r, g, b;
} state_led_entry_t;

// clang-format off
static const state_led_entry_t state_led_table[] = {
  [STATE_RESET]         = { -1,              0,   0,   0   },
  [STATE_BOOTING]       = { STATE_RESET,     255, 0,   0   },
  [STATE_SYS_INIT]      = { STATE_BOOTING,   0,   255, 0   },
  [STATE_BLE_INIT]      = { STATE_SYS_INIT,  0,   0,   255 },
  [STATE_INIT_COMPLETE] = { STATE_BLE_INIT,  255, 255, 0   },
  [STATE_BLE_WAITING]   = { -1,              0,   255, 255 },
  [STATE_BLE_CONNECTED] = { -1,              0,   0,   0   },
  [STATE_USB_CONNECTED] = { -1,              0,   0,   0   },
  [STATE_BOOTLOADER]    = { -1,              255, 255, 255 },
};
// clang-format on

volatile static state_system_t current_state = STATE_RESET;
volatile static connection_preference_t conn_pref = CONN_PREF_USB;

/**
 * @brief システムの優先接続モードを設定する。
 * @param pref 設定する優先接続モード
 */
void state_set_connection_preference(connection_preference_t pref) {
  conn_pref = pref;
}

/**
 * @brief システムの優先接続モードを取得する。
 * @return 現在の優先接続モード
 */
connection_preference_t state_get_connection_preference(void) {
  return conn_pref;
}

/**
 * @brief 優先接続モードを切り替える。
 * @param pref 切り替える優先接続モード
 */
void state_switch_connection(connection_preference_t pref) {
  conn_pref = pref;
  // BLE優先に切り替えた場合、BLEが有効でなければ有効にする
  if (pref == CONN_PREF_BLE) {
    if (!ble_is_enabled()) {
      ble_power_set(true);
    }
    state_set_system(ble_is_connected() ? STATE_BLE_CONNECTED
                                        : STATE_BLE_WAITING);
  }
}

/**
 * @brief システム状態を設定する。
 * @param new_state 設定する新しい状態
 */
void state_set_system(state_system_t new_state) {
  const state_led_entry_t *entry = &state_led_table[new_state];

  if (entry->required_from >= 0 &&
      current_state != (state_system_t)entry->required_from) {
    return;
  }

  led_put_rgb(entry->r, entry->g, entry->b);
  current_state = new_state;

  switch (new_state) {
  case STATE_RESET:
    watchdog_reboot(0, 0, 0);
    watchdog_enable(0, 1);
    return;
  case STATE_BOOTLOADER:
    reset_usb_boot(0, 0);
    return;
  default:
    return;
  }
}

/**
 * @brief システムの現在の状態を取得する。
 * @return 現在のシステム状態
 */
state_system_t state_get_system(void) { return current_state; }
