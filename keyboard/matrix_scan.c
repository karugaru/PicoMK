#include "matrix_scan.h"
#include "../settings/board.h"
#include "../settings/keymap.h"
#include "../settings/settings.h"
#include "../settings/users.h"

#include "event.h"
#include <stdio.h>

#ifndef DEBUG_MATRIX_SCAN
#define DEBUG_MATRIX_SCAN 0
#endif

#ifndef DEBUG_MATRIX_SCAN_DEEP
#define DEBUG_MATRIX_SCAN_DEEP 0
#endif

static bool scan_each_lines();
static bool scan_line(int row);

//----------------------------------------------------------------
// 静的変数
//----------------------------------------------------------------

// デバウンス後の前回のキーの状態
static volatile bool prev_gpio_state[ROWS][COLS] = {false};
// デバウンス前の最新のキーの状態
static volatile bool last_gpio_state[ROWS][COLS] = {false};
// 最後に状態が変化した時刻
static volatile absolute_time_t last_change_time = 0;

//----------------------------------------------------------------
// 関数定義
//----------------------------------------------------------------

/**
 * @brief キーマトリクススキャンの初期化を行います。
 *        各行ピンを入力モードに設定し、各列ピンをプルアップ付き入力モードに設定します。
 */
void matrix_init(void) {
  // GPIO初期化

  // すべての行のピンをプル抵抗を無効にして入力モードに設定
  for (int i = 0; i < ROWS; i++) {
    uint8_t pin = rows_pins[i];

    gpio_set_dir(pin, GPIO_IN);
    gpio_put(pin, 0);
    gpio_set_function(pin, GPIO_FUNC_SIO);
    gpio_disable_pulls(pin);
  }

  // すべての列のピンをプルアップありで入力モードに設定
  for (int i = 0; i < COLS; i++) {
    uint8_t pin = cols_pins[i];

    gpio_set_dir(pin, GPIO_IN);
    gpio_put(pin, 0);
    gpio_set_function(pin, GPIO_FUNC_SIO);
    gpio_pull_up(pin);
  }
}

/**
 * @brief デバウンス付きマトリクススキャンを行います。
 *        マトリクス全体をスキャンし、状態が変化した場合は最後の変化時刻を更新します。
 *        最後の状態変化から指定時間以上経過している場合、状態の変化を確定し、前回の状態を更新します。
 *        また、状態の変化をイベントとして処理します。
 *
 *        ユーザー定義イベントコールバック関数が設定されている場合、最初にそれを呼び出します。
 *        続いて標準のイベント処理を呼び出します。
 *
 * @param event_user_callback ユーザー定義イベントコールバック関数
 */
void matrix_process(void) {
  // 現在時刻を取得
  absolute_time_t current_time = get_absolute_time();

  // すべての行をスキャン
  bool matrix_changed = scan_each_lines();

  // 状態が変化した場合
  if (matrix_changed) {
    last_change_time = current_time;

#if DEBUG_MATRIX_SCAN_DEEP
    printf("Matrix[\n");
    for (int row = 0; row < ROWS; row++) {
      printf("  ");
      for (int col = 0; col < COLS; col++) {
        printf("%d", last_gpio_state[row][col] ? 1 : 0);
      }
      printf("\n");
    }
    printf("]\n");
#endif
  }

  // 最後の状態変化から指定時間以上経過しているかチェック
  if (absolute_time_diff_us(last_change_time, current_time) <
      DEBOUNCE_TIME_MS * 1000) {
    return;
  }

  // 状態の変化を確定し、前回の状態を更新
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      // 状態が変化していればイベント処理
      if (last_gpio_state[row][col] != prev_gpio_state[row][col]) {

        icode_t icode = icode_lookup(row, col);
        bool pressed = last_gpio_state[row][col];

#if DEBUG_MATRIX_SCAN
        printf("Matrix debounced 0x%04lX (%d, %d) %s\n", icode, row, col,
               pressed ? "pressed" : "released");
#endif

        // ユーザー定義イベントコールバックを呼び出し
        bool process_subsequent = users_event_callback(&icode, pressed);
        // 標準のイベント処理を呼び出し
        if (process_subsequent) {
          event_process_standard(icode, pressed);
        }
      }
      // 前回の状態を更新
      prev_gpio_state[row][col] = last_gpio_state[row][col];
    }
  }
}

//----------------------------------------------------------------
// 静的関数
//----------------------------------------------------------------

/**
 * @brief マトリクス全体をスキャンし、キーの状態を更新します。
 *       各行を順にスキャンし、状態の変化があればtrueを返します。
 * @return bool 一つ以上のキーの状態が変更されている場合にtrueを返します。
 */
static bool scan_each_lines() {
  bool any_changed = false;
  // 各行を順にスキャン
  for (int row = 0; row < ROWS; row++) {
    if (scan_line(row)) {
      any_changed = true;
    }
  }
  return any_changed;
}

/**
 * @brief 指定された行をスキャンし、キーの状態を更新します。
 *        選択行のピンをLOWに設定し、各列の状態を読み取ります。
 *        状態が変化した場合、trueを返します。
 *        スキャン後、選択行のピンを入力モードに戻します。
 * @param row スキャンする行番号 (0からROWS-1)
 * @return bool 一つ以上のキーの状態が変更されている場合にtrueを返します。
 */
static bool scan_line(int row) {
  bool changed = false;

  // 選択行のピンをLOWで出力モードに設定
  gpio_set_dir(rows_pins[row], GPIO_OUT);
  sleep_us(PIN_SETTLE_TIME_US);

  // 各列を読み取り
  for (int col = 0; col < COLS; col++) {
    // 列はプルアップ入力。押下時は0になる想定。
    bool pressed = (gpio_get(cols_pins[col]) == 0);
    if (last_gpio_state[row][col] != pressed) {
      changed = true;
      last_gpio_state[row][col] = pressed;
    }
  }

  // 選択行のピンを入力(ハイインピーダンス)へ戻す
  gpio_set_dir(rows_pins[row], GPIO_IN);
  return changed;
}
