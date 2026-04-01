#include <pico/unique_id.h>
#include <stddef.h>
#include <string.h>
#include <tusb.h>

#include "../hid/hid.h"
#include "usb_descriptors.h"

// デバイスディスクリプタ
static tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = USB_BCD,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = USB_VID,
    .idProduct = USB_PID,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01,
};

uint8_t const *tud_descriptor_device_cb(void) {
  return (uint8_t const *)&desc_device;
}

// HIDレポートディスクリプタ
uint8_t const *tud_hid_descriptor_report_cb(uint8_t _instance) {
  // hid/hid.c に定義された既存のHIDディスクリプタを再利用する
  return hid_descriptor;
}

enum { ITF_NUM_HID, ITF_NUM_TOTAL };

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID 0x81

// コンフィグレーションディスクリプタ
// HIDレポートディスクリプタのサイズはコンパイル時に不明のため、
// 実行時にコンフィグレーションディスクリプタのサイズフィールドを修正する
static uint8_t desc_configuration[] = {
    // Config number, interface count, string index, total length, attribute,
    // power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    // ※ report descriptor lenは後から修正する（暫定値0を使用）
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, 0, EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 1),
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t _index) {
  return desc_configuration;
}

enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
};

// 文字列ディスクリプタ
static char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // 0: English (0x0409)
    "PicoMK",                   // 1: Manufacturer
    "PicoMK",                   // 2: Product
    NULL,                       // 3: Serial (pico_unique_idで自動生成)
};

static uint16_t _desc_str[32 + 1];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;
  size_t chr_count;

  switch (index) {
  case STRID_LANGID:
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
    break;

  case STRID_SERIAL: {
    // Picoのユニークボード識別子をシリアルとして使用
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    chr_count = 0;
    for (size_t i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; i++) {
      uint8_t byte = board_id.id[i];
      uint8_t hi = (byte >> 4) & 0x0F;
      uint8_t lo = byte & 0x0F;
      _desc_str[1 + chr_count++] = hi < 10 ? ('0' + hi) : ('A' + hi - 10);
      _desc_str[1 + chr_count++] = lo < 10 ? ('0' + lo) : ('A' + lo - 10);
    }
    break;
  }

  default:
    if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) {
      return NULL;
    }

    const char *str = string_desc_arr[index];
    chr_count = strlen(str);
    size_t const max_count =
        sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
    if (chr_count > max_count) {
      chr_count = max_count;
    }

    // ASCII → UTF-16変換
    for (size_t i = 0; i < chr_count; i++) {
      _desc_str[1 + i] = str[i];
    }
    break;
  }

  // 先頭バイト: 長さ(ヘッダ含む)、2バイト目: 文字列型
  _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

  return _desc_str;
}

void usb_descriptors_init(void) {
  // HID descriptor内のwReportLengthフィールドのオフセットを計算
  const size_t offset = TUD_CONFIG_DESC_LEN + sizeof(tusb_desc_interface_t) +
                        offsetof(tusb_hid_descriptor_hid_t, wReportLength);
  uint16_t report_desc_len = hid_descriptor_len;
  desc_configuration[offset] = report_desc_len & 0xFF;
  desc_configuration[offset + 1] = (report_desc_len >> 8) & 0xFF;
}
