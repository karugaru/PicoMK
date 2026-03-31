#include <btstack.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "../keyboard/event.h"
#include "../state/state.h"
#include "advertising_data.h"
#include "ble.h"
#include "picomk.h"

#ifndef DEBUG_BLE
#define DEBUG_BLE 0
#endif

#if DEBUG_BLE
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

// --------------------------------
// 関数宣言
// --------------------------------
static void packet_handler(uint8_t packet_type, uint16_t channel,
                           uint8_t *packet, uint16_t size);
static void send_report();

// --------------------------------
// BLE系変数定義
// --------------------------------
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;

static uint8_t battery = 100;
static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;
static bool ble_enabled = false;

// --------------------------------
// 関数定義
// --------------------------------

/**
 * @brief BLEの初期化を行う。
 */
void ble_setup(void) {
  // Initialize L2CAP
  l2cap_init();

  // Initialize Security Manager
  sm_init();
  sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
  sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION |
                                     SM_AUTHREQ_BONDING);

  // Initialize GATT Server
  att_server_init(profile_data, NULL, NULL);

  // Initialize GATT Services
  battery_service_server_init(battery);
  device_information_service_server_init();

  // Initialize HID Device
  hids_device_init(0, hid_descriptor, hid_descriptor_len);

  // Initialize Advertisement
  uint16_t adv_int_min = 0x0030;
  uint16_t adv_int_max = 0x0030;
  uint8_t adv_type = 0;
  bd_addr_t null_addr;
  memset(null_addr, 0, 6);
  gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0,
                                null_addr, 0x07, 0x00);
  gap_advertisements_set_data(adv_data_len, (uint8_t *)adv_data);
  gap_advertisements_enable(1);

  // Initialize Event Handlers
  hci_event_callback_registration.callback = &packet_handler;
  hci_add_event_handler(&hci_event_callback_registration);
  sm_event_callback_registration.callback = &packet_handler;
  sm_add_event_handler(&sm_event_callback_registration);
  hids_device_register_packet_handler(packet_handler);
}

/**
 * @brief btstackの電源モードを設定する。
 *        OFF時は接続中のBLEも切断される。
 * @param power trueでON、falseでOFF
 */
void ble_power_set(bool power) {
  if (power == ble_enabled) {
    return;
  }

  ble_enabled = power;
  if (power) {
    hci_power_control(HCI_POWER_ON);
  } else {
    if (con_handle != HCI_CON_HANDLE_INVALID) {
      gap_disconnect(con_handle);
    }
    hci_power_control(HCI_POWER_OFF);
  }
}

/**
 * @brief btstackの処理ワーカーを起動する。
 */
void ble_poll(void) {
    btstack_run_loop_poll_data_sources_from_irq();
}

/**
 * @brief BLEが接続中かどうかを返す。
 * @return BLE接続中はtrue、それ以外はfalse
 */
bool ble_is_connected(void) { return con_handle != HCI_CON_HANDLE_INVALID; }

/**
 * @brief BLEが有効かどうかを返す。
 * @return BLE有効時はtrue、それ以外はfalse
 */
bool ble_is_enabled(void) { return ble_enabled; }

/**
 * @brief BLE HIDレポートの送信許可を要求する。
 *        btstackのコールバックで実際の送信が行われる。
 */
void ble_request_can_send(void) {
  if (con_handle != HCI_CON_HANDLE_INVALID) {
    hids_device_request_can_send_now_event(con_handle);
  }
}

// ---------------------------------
// 静的関数
// ---------------------------------

/**
 * @brief btstackのイベントハンドラ
 * @param packet_type パケットの種類
 * @param channel チャンネル
 * @param packet パケットデータ
 * @param size パケットサイズ
 */
static void packet_handler(uint8_t packet_type, uint16_t channel,
                           uint8_t *packet, uint16_t size) {
  UNUSED(channel);
  UNUSED(size);

  if (packet_type != HCI_EVENT_PACKET)
    return;

  switch (hci_event_packet_get_type(packet)) {
  case HCI_EVENT_DISCONNECTION_COMPLETE:
    con_handle = HCI_CON_HANDLE_INVALID;
    DEBUG_PRINT("Disconnected\n");
    state_set_system(STATE_BLE_WAITING);
    break;
  case SM_EVENT_JUST_WORKS_REQUEST:
    DEBUG_PRINT("Just Works requested\n");
    sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
    break;
  case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
    DEBUG_PRINT("Confirming numeric comparison: %" PRIu32 "\n",
                sm_event_numeric_comparison_request_get_passkey(packet));
    sm_numeric_comparison_confirm(
        sm_event_passkey_display_number_get_handle(packet));
    break;
  case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
    DEBUG_PRINT("Display Passkey: %" PRIu32 "\n",
                sm_event_passkey_display_number_get_passkey(packet));
    break;
  case HCI_EVENT_HIDS_META:
    switch (hci_event_hids_meta_get_subevent_code(packet)) {
    case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
      con_handle = hids_subevent_input_report_enable_get_con_handle(packet);
      DEBUG_PRINT("Report Characteristic Subscribed %u\n",
                  hids_subevent_input_report_enable_get_enable(packet));
      state_set_system(STATE_BLE_CONNECTED);
      break;
    case HIDS_SUBEVENT_CAN_SEND_NOW:
      send_report();
      break;
    default:
      DEBUG_PRINT("Unhandled HIDS subevent 0x%02X\n",
                  hci_event_hids_meta_get_subevent_code(packet));
      break;
    }
    break;

  default:
    break;
  }
}

/**
 * @brief HIDレポートを送信する。
 *        送信すべきレポートがある場合にのみ送信する。
 */
static void send_report() {
  uint8_t report[HID_REPORT_SIZE_MAX] = {0};

  if (event_pop_keyboard_report(report)) {
    hids_device_send_input_report_for_id(con_handle, KEYBOARD_REPORT_ID, report,
                                         HID_KEYBOARD_REPORT_SIZE);
  }

  if (event_pop_consumer_report(report)) {
    hids_device_send_input_report_for_id(con_handle, CONSUMER_REPORT_ID, report,
                                         HID_CONSUMER_REPORT_SIZE);
  }

  if (event_pop_mouse_report(report)) {
    hids_device_send_input_report_for_id(con_handle, MOUSE_REPORT_ID, report,
                                         HID_MOUSE_REPORT_SIZE);
  }
}
