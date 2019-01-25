#ifndef __SERIAL_H__
#define __SERIAL_H__

extern bool serial_read(char** msg, int* length);
extern void serial_write(char* msg, int length);

#endif//__SERIAL_H__
