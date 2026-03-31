#include <string.h>
#include <tusb.h>

#include "../hid/hid.h"
#include "../keyboard/event.h"
#include "../state/state.h"
#include "usb_descriptors.h"
#include "usb_hid.h"

static void usb_hid_send_report(uint8_t start_report_id);

// --------------------------------
// 公開関数
// --------------------------------

/**
 * @brief USB HIDデバイスの初期化を行う。
 */
void usb_hid_init(void) {
  usb_descriptors_init();
  tud_init(BOARD_TUD_RHPORT);
}

/**
 * @brief USB HIDのデバイスのタスク処理を行う。
 *        メインループから定期的に呼び出す必要がある。
 */
void usb_hid_task(void) { tud_task(); }

/**
 * @brief USB HIDレポートの送信を試みる。
 *        USBがマウント済みかつHIDが準備完了の場合にレポートチェーンを開始する。
 */
void usb_hid_send_reports(void) {
  if (!tud_mounted() || !tud_hid_ready() || !event_has_event()) {
    return;
  }
  usb_hid_send_report(KEYBOARD_REPORT_ID);
}

/**
 * @brief USBがマウントされており、HIDが有効かどうかを返す。
 *        USB接続中にBLEよりUSBを優先するために使用する。
 * @return USBマウント中はtrue、それ以外はfalse
 */
bool usb_hid_is_active(void) { return tud_mounted(); }

// --------------------------------
// TinyUSBデバイスコールバック
// --------------------------------

/**
 * @brief USBデバイスがマウントされた時のコールバック。
 */
void tud_mount_cb(void) { state_set_system(STATE_USB_CONNECTED); }

/**
 * @brief USBデバイスがアンマウントされた時のコールバック。
 */
void tud_umount_cb(void) { state_set_system(STATE_BLE_WAITING); }

/**
 * @brief USBバスがサスペンドされた時のコールバック。
 */
void tud_suspend_cb(bool remote_wakeup_en) { (void)remote_wakeup_en; }

/**
 * @brief USBバスがレジュームされた時のコールバック。
 */
void tud_resume_cb(void) {}

// --------------------------------
// TinyUSB HIDコールバック
// --------------------------------

/**
 * @brief HIDレポート送信完了コールバック。
 *        レポートチェーンの次のレポートを送信する。
 */
void tud_hid_report_complete_cb(uint8_t _instance, uint8_t const *report,
                                uint16_t _len) {
  // report[0]はレポートID
  uint8_t next_report_id = report[0] + 1;
  if (next_report_id <= REPORT_ID_MAX) {
    usb_hid_send_report(next_report_id);
  }
}

/**
 * @brief GET_REPORTリクエストのコールバック。
 */
uint16_t tud_hid_get_report_cb(uint8_t _instance, uint8_t _report_id,
                               hid_report_type_t _report_type, uint8_t *_buffer,
                               uint16_t _reqlen) {
  return 0;
}

/**
 * @brief SET_REPORTリクエストのコールバック。
 *        キーボードLED（CapsLock等）の処理に使用可能。
 */
void tud_hid_set_report_cb(uint8_t _instance, uint8_t _report_id,
                           hid_report_type_t _report_type,
                           uint8_t const *_buffer, uint16_t _bufsize) {}

// --------------------------------
// 内部関数
// --------------------------------

/**
 * @brief 指定されたレポートIDからレポートチェーンを開始する。
 *        送信すべきレポートが見つかるまで順に探索し、
 *        見つかったら送信して終了する（続きはtud_hid_report_complete_cbで処理）。
 * @param start_report_id 開始レポートID
 */
static void usb_hid_send_report(uint8_t start_report_id) {
  uint8_t report[HID_REPORT_SIZE_MAX] = {0};

  for (uint8_t id = start_report_id; id <= REPORT_ID_MAX; id++) {
    memset(report, 0, sizeof(report));

    switch (id) {
    case KEYBOARD_REPORT_ID:
      if (event_pop_keyboard_report(report)) {
        tud_hid_report(KEYBOARD_REPORT_ID, report, HID_KEYBOARD_REPORT_SIZE);
        return;
      }
      break;

    case MOUSE_REPORT_ID:
      if (event_pop_mouse_report(report)) {
        tud_hid_report(MOUSE_REPORT_ID, report, HID_MOUSE_REPORT_SIZE);
        return;
      }
      break;

    case CONSUMER_REPORT_ID:
      if (event_pop_consumer_report(report)) {
        tud_hid_report(CONSUMER_REPORT_ID, report, HID_CONSUMER_REPORT_SIZE);
        return;
      }
      break;
    }
  }
}
