#ifndef MEMORY_BUFFER_H
#define MEMORY_BUFFER_H

#include <stdint.h>

struct _MemoryBuffer
{
	// size of each memory slot
	// number of slots
	// next slot to be allocated
	size_t slotSize, numSlots, currentSlot;

	// boolean indicator, tells whether a slot is taken
	uint8_t* occupied;

	// slot space
	void* memSpace;
};
typedef struct _MemoryBuffer* MemoryBuffer;

MemoryBuffer createMemoryBuffer(size_t numSlots, size_t bytesPerSlot);
void destroyMemoryBuffer(MemoryBuffer m);
void* allocMemoryBufferSlot(MemoryBuffer m);
uint8_t freeMemoryBufferSlot(MemoryBuffer m, void* slot);

#endif