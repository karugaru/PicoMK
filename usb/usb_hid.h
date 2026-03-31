#ifndef USB_HID_H
#define USB_HID_H

void usb_hid_init(void);
void usb_hid_task(void);
void usb_hid_send_reports(void);
bool usb_hid_is_active(void);

#endif // USB_HID_H
