#include "config.h"
#include "heap.h"

//GOSUB�n�̃q�[�v�̈�
static byte heap_memory[MAX_GOSUB_HEAP_SIZE];
static byte heap_idx = 0;

//�q�[�v�������ɃG���L���[
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

//�q�[�v�̈悩��f�L���[
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