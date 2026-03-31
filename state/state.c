#include <hardware/watchdog.h>
#include <pico/bootrom.h>

#include "../led/led.h"
#include "state.h"

volatile static state_system_t current_state = STATE_RESET;

void state_set_system(state_system_t new_state) {

  switch (new_state) {
  case STATE_RESET:
    led_put_rgb(0, 0, 0);
    watchdog_reboot(0, 0, 0);
    watchdog_enable(0, 1);
    current_state = new_state;
    return;
  case STATE_BOOTING:
    if (current_state != STATE_RESET) {
      return;
    }
    led_put_rgb(255, 0, 0);
    current_state = new_state;
    return;
  case STATE_SYS_INIT:
    if (current_state != STATE_BOOTING) {
      return;
    }
    led_put_rgb(0, 255, 0);
    current_state = new_state;
    return;
  case STATE_BLE_INIT:
    if (current_state != STATE_SYS_INIT) {
      return;
    }
    led_put_rgb(0, 0, 255);
    current_state = new_state;
    return;
  case STATE_INIT_COMPLETE:
    if (current_state != STATE_BLE_INIT) {
      return;
    }
    led_put_rgb(255, 255, 0);
    current_state = new_state;
    return;
  case STATE_BLE_WAITING:
    led_put_rgb(0, 255, 255);
    current_state = new_state;
    return;
  case STATE_BLE_CONNECTED:
    led_put_rgb(0, 0, 0);
    current_state = new_state;
    return;
  case STATE_USB_CONNECTED:
    led_put_rgb(0, 0, 0);
    current_state = new_state;
    return;
  case STATE_BOOTLOADER:
    current_state = new_state;
    led_put_rgb(255, 255, 255);
    reset_usb_boot(0, 0);
    return;
  }
}

state_system_t state_get_system(void) { return current_state; }
