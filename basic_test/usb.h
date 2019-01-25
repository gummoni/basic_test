#ifndef __USB_H__
#define __USB_H__

extern bool usb_read(char** msg, int* length);
extern void usb_write(char* msg, int length);

#endif//__SERIAL_H__
