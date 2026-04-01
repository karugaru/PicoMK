#ifndef PICOMK_USB_DESCRIPTORS_H
#define PICOMK_USB_DESCRIPTORS_H

// USB VID/PID (開発用)
#define USB_VID 0xCafe
#define USB_PID 0x4001
#define USB_BCD 0x0200

/**
 * @brief
 * USBをHID機器として使用する際のコンフィグレーションディスクリプタを初期化する。
 */
void usb_descriptors_init(void);

#endif // PICOMK_USB_DESCRIPTORS_H
