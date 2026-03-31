#ifndef STATE_H
#define STATE_H

typedef enum {
  STATE_RESET,
  STATE_BOOTING,
  STATE_SYS_INIT,
  STATE_BLE_INIT,
  STATE_INIT_COMPLETE,
  STATE_BLE_WAITING,
  STATE_BLE_CONNECTED,
  STATE_USB_CONNECTED,
  STATE_BOOTLOADER,
} state_system_t;

void state_set_system(state_system_t new_state);
state_system_t state_get_system(void);

#endif // STATE_H
