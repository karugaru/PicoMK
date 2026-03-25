#include "code.h"

/**
 * @brief 内部キーコードからキーボード修飾子コードに変換
 * @return 変換後のキーボード修飾子コード、該当しない場合はKMC_UNDEFINED
 */
keyboard_modifier_t code_icode_to_modifier(icode_t ic) {
  switch (ic) {
  case IMKC_LEFT_CONTROL:
    return KMC_LEFT_CONTROL;
  case IMKC_LEFT_SHIFT:
    return KMC_LEFT_SHIFT;
  case IMKC_LEFT_ALT:
    return KMC_LEFT_ALT;
  case IMKC_LEFT_GUI:
    return KMC_LEFT_GUI;
  case IMKC_RIGHT_CONTROL:
    return KMC_RIGHT_CONTROL;
  case IMKC_RIGHT_SHIFT:
    return KMC_RIGHT_SHIFT;
  case IMKC_RIGHT_ALT:
    return KMC_RIGHT_ALT;
  case IMKC_RIGHT_GUI:
    return KMC_RIGHT_GUI;
  default:
    return KMC_UNDEFINED;
  }
}

/**
 * @brief 内部キーコードからキーボード修飾子ビットを抽出
 * @return 抽出した修飾子ビット、該当しない場合はKMC_UNDEFINED
 */
keyboard_modifier_bits_t code_icode_extract_modifier_bits(icode_t ic) {
  if (ic < ICODE_STANDARD_START || ic > ICODE_STANDARD_END) {
    return KMC_UNDEFINED;
  }

  // この時点でicは16bit幅であることが確定しているので、
  // 下位8bitがキーコード、上位8bitが修飾子ビットを表す
  keyboard_modifier_bits_t mod_bits = (ic & 0xFF00) >> 8;
  return mod_bits;
}

/**
 * @brief 内部キーコードからコンシューマーコードに変換
 * @return 変換後のコンシューマーコード、該当しない場合はCC_UNDEFINED
 */
consumer_code_t code_icodes_to_consumer(icode_t ic) {
  switch (ic) {
  case ICC_RECORD:
    return CC_RECORD;
  case ICC_FAST_FORWARD:
    return CC_FAST_FORWARD;
  case ICC_REWIND:
    return CC_REWIND;
  case ICC_NEXT_TRACK:
    return CC_NEXT_TRACK;
  case ICC_PREV_TRACK:
    return CC_PREV_TRACK;
  case ICC_STOP_TRACK:
    return CC_STOP_TRACK;
  case ICC_EJECT:
    return CC_EJECT;
  case ICC_RANDOM_PLAY:
    return CC_RANDOM_PLAY;
  case ICC_STOP_EJECT:
    return CC_STOP_EJECT;
  case ICC_PLAY_PAUSE:
    return CC_PLAY_PAUSE;
  case ICC_VOL_MUTE:
    return CC_VOL_MUTE;
  case ICC_VOL_UP:
    return CC_VOL_UP;
  case ICC_VOL_DOWN:
    return CC_VOL_DOWN;
  default:
    return CC_UNDEFINED;
  }
}

/**
 * @brief 内部キーコードからマウスボタンコードに変換
 * @return 変換後のマウスボタンコード、該当しない場合はMBC_UNDEFINED
 */
mouse_button_code_t code_icodes_to_mouse_button(icode_t ic) {
  switch (ic) {
  case IMC_MOUSE_LEFT:
    return MBC_LEFT;
  case IMC_MOUSE_RIGHT:
    return MBC_RIGHT;
  case IMC_MOUSE_MIDDLE:
    return MBC_MIDDLE;
  default:
    return MBC_UNDEFINED;
  }
}
