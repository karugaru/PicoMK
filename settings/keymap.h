#ifndef PICOMK_KEYMAP_H
#define PICOMK_KEYMAP_H

#include "../keyboard/code.h"
#include "board.h"

//--------------------------------
// 関数宣言
//--------------------------------

void keyswitch_index_init(void);
icode_t icode_lookup(uint8_t row, uint8_t col);

//--------------------------------
// キーマップ設定
//--------------------------------

// ユーザー定義キーコード
// enum user_key { IUC_WHEELMODE = ICODE_USER_START };
#define IUC_BLACKSCREEN LEFT_ALT(LEFT_CTRL(IKC_F10))

// clang-format off
/**
 * キーマップ
 * LAYOUTで定義されたスイッチに対応するキーコードを定義する。
 */
#define KEYMAP {                                           \
    ISC_BOOT,         IKC_SPACE,        IUC_BLACKSCREEN,   \
    IMKC_LEFT_CONTROL,IMKC_LEFT_SHIFT,  IMKC_LEFT_ALT,     \
    IKC_NOOP,         IKC_NOOP,         IMKC_LEFT_GUI,     \
    IMC_MOUSE_LEFT,   IMC_MOUSE_MIDDLE, IMC_MOUSE_RIGHT,   \
                                                           \
    ICC_REWIND,       IKC_UP_ARROW,     ICC_FAST_FORWARD,  \
    IKC_LEFT_ARROW,   ICC_PLAY_PAUSE,   IKC_RIGHT_ARROW,   \
    ICC_PREV_TRACK,   IKC_DOWN_ARROW,   ICC_NEXT_TRACK,    \
}
// clang-format on

#endif // PICOMK_KEYMAP_H
