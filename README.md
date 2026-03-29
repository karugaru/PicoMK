# PicoMK (Pico mechanical keyboard firmware)

- PicoMKは、Raspberry Pi Pico W上で動作するキーボードファームウェアです。
- キーボード、マウス、リモートコントローラーなどの機能を持つ多機能入力デバイス用ファームウェアです。
- カスタマイズ可能なキー配置と多様な入力方法を提供します。

## 特徴

### 他のファームウェアより優れている点

- Raspberry Pi Pico W による オンボードチップのBLEを使用したワイヤレス接続が標準でサポートされています。(有線接続はサポートされていません。)
- これはQMKやZMK、KMKなどの他の主要なキーボードファームウェアではサポートされていません。(2025年8月現在)
- Pico-SDK以外の外部ライブラリに依存していません。
- すべてのコードのライセンスはMITライセンスで、商用利用も可能です。

### 他のファームウェアに劣っている点

- 上記以外ほぼ全てです。
- 他のファームウェアライブラリではライセンスが厳しい、商用利用できない、外部ライブラリに依存している、BLE通信にプロプライエタリなコードが必要であるなどの制約がありますが、
  PicoMKはそれらの制約を回避することを目的としています。
- 逆に、Picoで有線接続を使用する場合や、他のマイコンを使用する場合は、上記のようなファームウェアを使用するべきです。

## ビルド手順

- ビルドするにはこのリポジトリをクローンし、vscodeの拡張機能の[Raspberry Pi Pico](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico)を使用する必要があります。

## 使い方

- まだ開発中のため、ドキュメントは不完全です。詳しい使用方法はソースコードを直接参照してください。
- 物理ボードに対する設定は[settings/board.h](settings/board.h)で行います。
- キーマップの定義は、[settings/keymap.h](settings/keymap.h)で行います。使用できるキーコードは[keyboard/code.h](keyboard/code.h)に定義されています。
- マウスキーやトラックパッドなどの設定は[settings/settings.h](settings/settings.h)で行います。
- その他固有の設定は[main.c](main.c)を参照してください。
- デフォルトの設定（及び実装）では、キーマトリクスとトラックパッド(cirque pinnable)とフルカラーLED（WS2812B）を使用する前提となっています。
