#ifndef BLE_H
#define BLE_H

#include "../hid/hid.h"

void ble_setup(void);
void ble_power_set(bool power);
void ble_invoke_check_event(void);

#endif // BLE_H
