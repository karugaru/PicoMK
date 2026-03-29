#ifndef SETTING_H
#define SETTING_H

#define LED_BRIGHTNESS 16 // LEDの明るさの最大値 (1-255)

#define DEBOUNCE_TIME_MS 5 // デバウンス時間

#define MOUSE_MOVE_DELTA 1    // マウスキーによる移動量
#define MOUSE_MOVE_THRESH 1   // マウス移動イベントとみなすための移動量の閾値
#define MOUSE_WHEEL_DELTA 1   // ホイール移動量の最大値
#define MOUSE_WHEEL_THRESH 64 // ホイール移動イベントとみなすための移動量の閾値

#define USE_PINNACLE 1 // Pinnacleトラックパッドの使用有無

#ifdef USE_PINNACLE

#include "../pinnacle/pinnacle.h"

// Pinnacleの回転設定。
#define PINNACLE_ROTATE PINNACLE_ROTATE_270

// 移動量はdelta^accel * speedで計算される。
#define PINNACLE_ACCEL 1.2f // Pinnacleの加速度設定 (0.5-2.0程度で調整)
#define PINNACLE_SPEED 0.8f // Pinnacleの速度設定 (0.5-2.0程度で調整)

#endif

void settings_init(void);

#endif // SETTING_H
