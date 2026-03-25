#include "users.h"

/**
 * @brief ユーザー定義イベントコールバック
 * @param icode
 * 押下または離上されたキーの内部コードへのポインタ。書き換えると後続の処理に影響を与える。
 * @param pressed 押下状態（true: 押下, false: 離上）
 * @return bool 後続の処理を行う場合にtrueを返します。
 */
bool users_event_callback(icode_t *icode, bool pressed) {
  // TODO もっと汎用的なイベント情報を渡すようにするほうが良いかも
  return true;
}
