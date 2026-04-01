#ifndef PICOMK_USB_HID_H
#define PICOMK_USB_HID_H

#include <stdbool.h>

void usb_hid_init(void);
void usb_hid_task(void);
void usb_hid_send_reports(void);
bool usb_hid_is_active(void);

#endif // PICOMK_USB_HID_H
