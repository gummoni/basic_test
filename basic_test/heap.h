#ifndef __HEAP_H__
#define __HEAP_H__

void heap_init(void);
bool heap_enqueue(byte value);
bool heap_dequeue(byte* value);

#endif//__HEAP_H__
