#ifndef PICOMK_BOARD_H
#define PICOMK_BOARD_H
#include <pico/stdlib.h>

//--------------------------------
// ボード設定
//--------------------------------
#define ROWS 5 // 行ピン数
#define COLS 5 // 列ピン数

#define ROWS_PINS {10, 11, 12, 13, 14} // 行ピン番号
#define COLS_PINS {21, 20, 19, 18, 17} // 列ピン番号

#define GPIO_SDA_PIN 4
#define GPIO_SCL_PIN 5
#define GPIO_DR_PIN 6

#define GPIO_LED_PIN 16

#define PIN_SETTLE_TIME_US 1 // ピン信号のセトルタイム

/**
 * レイアウト
 * 例として、{{1,1},{2,2},{3,3}}の場合、
 * KEYMAP[0]が行1列1のスイッチ、KEYMAP[1]が行2列2のスイッチ、
 * KEYMAP[2]が行3列3のスイッチに対応する。
 * 長さがROWS*COLSに満たない場合は、(-1,-1)で埋めること。
 */
// clang-format off
#define LAYOUT { {0,0},{0,1},{0,2},  \
                 {0,3},{0,4},{1,0},  \
                 {1,1},{1,2},{1,3},  \
                 {1,4},{2,0},{2,1},  \
                                     \
                 {2,2},{2,3},{2,4},  \
                 {3,0},{3,1},{3,2},  \
                 {3,3},{3,4},{4,0},  \
                 {-1,-1},{-1,-1},{-1,-1},{-1,-1}}
// clang-format on

//--------------------------------
// 変数宣言
//--------------------------------

extern const uint8_t rows_pins[ROWS];
extern const uint8_t cols_pins[COLS];
extern const uint8_t layout[ROWS * COLS][2];

#endif // PICOMK_BOARD_H
