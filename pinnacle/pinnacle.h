#ifndef PINNACLE_H
#define PINNACLE_H

#include "hardware/i2c.h"
#include <pico/stdlib.h>

// I2Cの通信速度
#define PINNACLE_I2C_BAUD 100000
// PinnacleのI2Cデバイスアドレス
#define PINNACLE_I2C_TARGET_ADDRESS 0x2A

// Pinnacle I2Cレジスタアドレス
#define PINNACLE_I2C_FIRMWARE_ID 0x00
#define PINNACLE_I2C_STATUS 0x02
#define PINNACLE_I2C_SYS_CONFIG 0x03
#define PINNACLE_I2C_FEED_CONFIG_1 0x04
#define PINNACLE_I2C_FEED_CONFIG_2 0x05
// #define PINNACLE_I2C_FEED_CONFIG_3 0x06
#define PINNACLE_I2C_CAL_CONFIG 0x07
#define PINNACLE_I2C_SAMPLE_RATE 0x09
#define PINNACLE_I2C_Z_IDLE 0x0A
// #define PINNACLE_I2C_Z_SCALER 0x0B
#define PINNACLE_I2C_SLEEP_INTERVAL 0x0C
#define PINNACLE_I2C_SLEEP_TIMER 0x0D
#define PINNACLE_I2C_PACKET_BYTE_0 0x12
// #define PINNACLE_I2C_PACKET_BYTE_1 0x13
// #define PINNACLE_I2C_PACKET_BYTE_2 0x14
// #define PINNACLE_I2C_PACKET_BYTE_3 0x15
// #define PINNACLE_I2C_PACKET_BYTE_4 0x16
// #define PINNACLE_I2C_PACKET_BYTE_5 0x17
#define PINNACLE_I2C_ERA_VALUE 0x1B
#define PINNACLE_I2C_ERA_ADDR_HIGH 0x1C
#define PINNACLE_I2C_ERA_ADDR_LOW 0x1D
#define PINNACLE_I2C_ERA_CONTROL 0x1E
// #define PINNACLE_I2C_HCO_ID 0x1F

/**
 * @brief Pinnacleのデータ構造体
 */
typedef struct {
  uint8_t buttons;
  int8_t xDelta;
  int8_t yDelta;
  int8_t wDelta;
} pinnacle_data_t;

/**
 * @brief 感度設定。
 *        感度が高いほど、同じ移動量に対して大きな移動量が報告される。
 */
typedef enum {
  PINNACLE_SENSITIVITY_MOST_SENSITIVE = 0,
  PINNACLE_SENSITIVITY_1 = 1,
  PINNACLE_SENSITIVITY_2 = 2,
  PINNACLE_SENSITIVITY_3 = 3,
  PINNACLE_SENSITIVITY_LEAST_SENSITIVE = 4,
} pinnacle_sensitivity_t;

/**
 * @brief 回転設定。
 *        トラックパッドの物理的な向きに合わせて設定する。
 *        例えば、トラックパッドが左に90度回転している場合はPINNACLE_ROTATE_270を指定する。
 */
typedef enum {
  PINNACLE_ROTATE_0,
  PINNACLE_ROTATE_90,
  PINNACLE_ROTATE_180,
  PINNACLE_ROTATE_270,
} pinnacle_rotate_t;

bool pinnacle_init(i2c_inst_t *i2c, uint8_t scl_pin, uint8_t sda_pin,
                   uint8_t dr_pin);
void pinnacle_set_speed(float accel, float speed);
void pinnacle_set_rotation(pinnacle_rotate_t rotation);
bool pinnacle_check_DR();
bool pinnacle_read_data(pinnacle_data_t *data);

#endif // PINNACLE_H

/*
 * メモ

 * PINNACLE_I2C_SLEEP_INTERVAL
 *   スリープモード中のチェック間隔。
 *   初期値は0x49(73)で、300ms毎にスリープから復帰して状態をチェックする。
 *   0x01あたり約4.1ms。
 *
 * PINNACLE_I2C_SLEEP_TIMER
 *   未入力時のスリープモード移行までの時間。
 *   初期値は0x27で、5000msで移行する。
 *   0x01あたり約128ms。
 */
