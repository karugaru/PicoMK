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

static async_at_time_worker_t picomk_worker;

/**
 * @brief async_contextの1ms定期ワーカーコールバック。
 *        事実上のメインループ処理。
 * @param context async_context
 * @param worker ワーカー
 */
static void picomk_worker_process(async_context_t *context,
                                  async_at_time_worker_t *worker) {
  // キーマトリクス処理を実行
  matrix_process();

  // 定期処理を実行
  event_process_periodic();

  // 接続モードとトランスポートの状態を取得
  connection_preference_t connection_pref = state_get_connection_preference();
  bool usb_active = usb_hid_is_active();
  bool ble_enabled = ble_is_enabled();
  bool ble_connected = ble_is_connected();

  // USB優先時、USB接続状態に応じてBLEを動的に制御
  if (connection_pref == CONN_PREF_USB) {
    if (usb_active && ble_enabled) {
      ble_power_set(false); // USB接続中はBLEをOFFにして省電力
    } else if (!usb_active && !ble_enabled) {
      ble_power_set(true); // USB未接続時はBLEをONにしてフォールバック
    }
  }

  // トランスポートの決定
  bool use_ble, use_usb;
  if (connection_pref == CONN_PREF_BLE) {
    use_ble = ble_connected;
    use_usb = !ble_connected && usb_active;
  } else {
    use_usb = usb_active;
    use_ble = !usb_active && ble_connected;
  }

  // USB定期処理
  if (!use_ble) {
    usb_hid_task();
  }

  // BLE定期処理
  if (ble_enabled) {
    ble_poll();
  }

  // 周辺機器のイベント処理（I2C通信を含むため必要な時のみ実行）
  if (peripheral_require_event_processing()) {
    peripheral_process_events();
  }

  // HIDイベントがあればアクティブなトランスポートにレポート送信を要求
  if (event_has_event()) {
    if (use_ble) {
      ble_request_can_send();
    } else if (use_usb) {
      usb_hid_send_reports();
    }
  }

  // 1ms後に再スケジュール
  async_context_add_at_time_worker_in_ms(context, worker, 1);
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

  // 初期化完了
  state_set_system(STATE_INIT_COMPLETE);
  state_refresh_runtime();

  // 定期処理ワーカーをasync_contextに登録
  picomk_worker.do_work = picomk_worker_process;
  async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(),
                                         &picomk_worker, 1);

  // メインループ
  while (true) {
    async_context_poll(cyw43_arch_async_context());
    async_context_wait_for_work_until(cyw43_arch_async_context(),
                                      at_the_end_of_time);
  }

  state_set_system(STATE_RESET);
  return 0;
}
