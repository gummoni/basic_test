#include "config.h"
#include "heap.h"

//GOSUB系のヒープ領域
static byte heap_memory[MAX_GOSUB_HEAP_SIZE];
static byte heap_idx = 0;

//ヒープメモリにエンキュー
bool heap_enqueue(byte value)
{
	if (heap_idx < MAX_GOSUB_HEAP_SIZE)
	{
		heap_memory[heap_idx++] = value;
		return true;
	}
	else
	{
		return false;
	}
}

//ヒープ領域からデキュー
bool heap_dequeue(byte* value)
{
	if (0 < heap_idx)
	{
		*value = heap_memory[--heap_idx];
		return true;
	}
	else
	{
		return false;
	}
}