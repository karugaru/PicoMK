#include <hardware/i2c.h>
#include <pico/bootrom.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>

#include "ble/ble.h"
#include "keyboard/code.h"
#include "keyboard/event.h"
#include "keyboard/matrix_scan.h"
#include "led/led.h"
#include "peripheral/peripheral.h"
#include "settings/board.h"
#include "settings/keymap.h"
#include "settings/settings.h"
#include "settings/users.h"
#include "state/state.h"
#include "usb/usb_hid.h"

#ifndef DEBUG_MAIN
#define DEBUG_MAIN 0
#endif

#if DEBUG_MAIN
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

static repeating_timer_t pico_timer;

/**
 * @brief
 * 一定周期で呼び出されるタイマ割り込みコールバック関数。
 * @param rt タイマ情報
 * @return true
 */
static bool pico_timer_callback(repeating_timer_t *rt) {
  // キーマトリクス処理を実行
  matrix_process();

  // 定期処理を実行
  event_process_periodic();

  // 内部HID状態に変更があれば、BLEにイベント処理を行うよう要求する
  if (event_has_event()) {
    ble_invoke_check_event();
  }

  // 周辺機器のイベント処理が必要なら、BLEにイベント処理を行うよう要求する
  if (peripheral_require_event_processing()) {
    ble_invoke_check_event();
  }
  return true;
}

/**
 * @brief メイン関数 エントリーポイント
 * @return 0
 */
int main() {
  state_set_system(STATE_BOOTING);
  stdio_init_all();

#if DEBUG_MAIN
  sleep_ms(2000); // UARTデバッグ用: 接続待ち
#endif
  DEBUG_PRINT("picomk v1 start\n");

  // LEDの初期化
  led_init(GPIO_LED_PIN, LED_BRIGHTNESS);
  state_set_system(STATE_SYS_INIT);

  // 設定の初期化
  settings_init();

  // マトリクススキャン初期化
  matrix_init();

  // イベント処理を初期化
  event_init();

  // BLEの初期化
  if (cyw43_arch_init()) {
    DEBUG_PRINT("failed to initialise cyw43_arch\n");
    return -1;
  }
  ble_setup();
  ble_power_set(true);
  state_set_system(STATE_BLE_INIT);

  // マトリクス以外の周辺機器の初期化
  peripheral_init();

  // USB HIDの初期化
  usb_hid_init();

  // 1ms周期でタイマ割り込みを設定
  add_repeating_timer_ms(1, pico_timer_callback, NULL, &pico_timer);

  // 初期化完了
  state_set_system(STATE_INIT_COMPLETE);

  // メインループ
  while (true) {
    // USB HIDタスク処理（TinyUSBデバイスタスク + レポート送信）
    usb_hid_task();

    // BLE処理
    async_context_poll(cyw43_arch_async_context());
    async_context_wait_for_work_until(cyw43_arch_async_context(),
                                      make_timeout_time_ms(10));
  }

  state_set_system(STATE_RESET);
  return 0;
}
