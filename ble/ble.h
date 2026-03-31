#ifndef BLE_H
#define BLE_H

#include "../hid/hid.h"

void ble_setup(void);
void ble_power_set(bool power);
void ble_poll(void);
bool ble_is_connected(void);
bool ble_is_enabled(void);
void ble_request_can_send(void);

#endif // BLE_H
