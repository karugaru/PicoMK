#include "advertising_data.h"
#include "btstack.h"

const uint8_t adv_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02,
    BLUETOOTH_DATA_TYPE_FLAGS,
    0x06,
    // Name
    0x0C,
    BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME,
    'R',
    'e',
    'm',
    'o',
    'p',
    'i',
    'c',
    'o',
    'n',
    ' ',
    ' ',
    // 16-bit Service UUIDs
    0x03,
    BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS,
    ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff,
    ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
    // Appearance HID - Generic Human Interface Device (Category 15,
    // Sub-Category 0)
    0x03,
    BLUETOOTH_DATA_TYPE_APPEARANCE,
    0xC0,
    0x03,
};
const uint8_t adv_data_len = sizeof(adv_data);
