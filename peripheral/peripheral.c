#include "peripheral.h"
#include "../hid/hid.h"
#include "../keyboard/event.h"
#include "../pinnacle/pinnacle.h"
#include "../settings/board.h"
#include "../settings/settings.h"

#ifndef DEBUG_PERIPHERAL
#define DEBUG_PERIPHERAL 0
#endif

#if DEBUG_PERIPHERAL
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

static int8_t pointing_device_pinnacle = -1;

/**
 * @brief 周辺機器の初期化を行う。
 * @return bool 初期化に成功した場合にtrueを返す。
 */
bool peripheral_init(void) {
  if (USE_PINNACLE) {
    // I2Cとトラックパッドの初期化
    if (!pinnacle_init(i2c0, GPIO_SDA_PIN, GPIO_SCL_PIN, GPIO_DR_PIN)) {
      DEBUG_PRINT("failed to initialise pinnacle\n");
      return false;
    }
    pinnacle_set_speed(PINNACLE_ACCEL, PINNACLE_SPEED);
    pinnacle_set_rotation(PINNACLE_ROTATE_270);
    pointing_device_pinnacle = event_request_pointing_device_id();
    if (pointing_device_pinnacle < 0) {
      DEBUG_PRINT("failed to request pointing device id for pinnacle\n");
      return false;
    }
  }
  return true;
}

/**
 * @brief 周辺機器のイベント処理が必要かどうかを取得する。
 * @return bool イベント処理が必要な場合にtrueを返す。
 */
bool peripheral_require_event_processing(void) {
  // トラックパッドのDRピンがアクティブならBLEにイベント処理を行うよう要求する
  if (USE_PINNACLE && pinnacle_check_DR()) {
    return true;
  }

  return false;
}

/**
 * @brief 周辺機器のイベント処理を行う。
 */
void peripheral_process_events(void) {
  if (USE_PINNACLE) {
    pinnacle_data_t tdata;
    while (pinnacle_read_data(&tdata)) {
      if (pointing_device_pinnacle >= 0) {
        event_accumulate_mouse(pointing_device_pinnacle, tdata.buttons,
                               tdata.xDelta, tdata.yDelta, tdata.wDelta);
      }
    }
  }
}
