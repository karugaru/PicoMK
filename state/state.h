#ifndef STATE_H
#define STATE_H

typedef enum {
  STATE_RESET,
  STATE_BOOTING,
  STATE_SYS_INIT,
  STATE_BLE_INIT,
  STATE_INIT_COMPLETE,
  STATE_USB_WAITING,
  STATE_BLE_WAITING,
  STATE_BLE_CONNECTED,
  STATE_USB_CONNECTED,
  STATE_BOOTLOADER,
} state_system_t;

typedef enum {
  CONN_PREF_BLE,
  CONN_PREF_USB,
} connection_preference_t;

void state_set_system(state_system_t new_state);
state_system_t state_get_system(void);

void state_set_connection_preference(connection_preference_t pref);
connection_preference_t state_get_connection_preference(void);

void state_switch_connection_preference(connection_preference_t pref);
void state_refresh_runtime(void);

#endif // STATE_H
