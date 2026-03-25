#include "keymap.h"
#include "board.h"
#include <stdio.h>

static const icode_t keymap[ROWS * COLS] = KEYMAP;
static size_t keyswitch_index_lookup[ROWS][COLS];

void keyswitch_index_init(void) {
  // keyswitch_index_lookupの初期化
  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      keyswitch_index_lookup[r][c] = -1; // 使用しないスイッチは-1に設定
    }
  }

  // LAYOUTとKEYMAPに基づいてkeyswitch_index_lookupを設定
  for (size_t i = 0; i < ROWS * COLS; i++) {
    uint8_t row = layout[i][0];
    uint8_t col = layout[i][1];
    if (row == (uint8_t)-1 || col == (uint8_t)-1) {
      continue; // 使用しないスイッチはスキップ
    }
    keyswitch_index_lookup[row][col] = i;
  }
}

/**
 * @brief 行と列からキーマップを参照し、キーコードを取得
 * @param row 行番号
 * @param col 列番号
 * @return キーコード
 */
icode_t icode_lookup(uint8_t row, uint8_t col) {
  size_t index = keyswitch_index_lookup[row][col];
  if (index != -1) {
    return keymap[index];
  }
  return IKC_NOOP; // 仮の戻り値
}