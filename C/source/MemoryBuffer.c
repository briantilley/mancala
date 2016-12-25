#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inc/MemoryBuffer.h"

// allocate space from OS
MemoryBuffer createMemoryBuffer(size_t numSlots, size_t bytesPerSlot)
{
	// make structure for metadata
	MemoryBuffer newBuf = (MemoryBuffer)malloc(sizeof(struct _MemoryBuffer));
	if(NULL == newBuf)
	{
		fprintf(stderr, "failed to create a memory buffer\n");
		exit(1);
	}

	// initialize members
	newBuf->slotSize = bytesPerSlot;
	newBuf->numSlots = numSlots;
	newBuf->currentSlot = 0;

	// allocate arrays for memory space and deletion indicators
	newBuf->occupied = (uint8_t*)malloc(numSlots * sizeof(uint8_t));
	if(NULL == newBuf->occupied)
	{
		fprintf(stderr, "failed to allocate deletion indicator array\n");
		exit(1);
	}
	// all slots start as unoccupied
	memset(newBuf->occupied, 0, numSlots);

	newBuf->memSpace = malloc(numSlots * bytesPerSlot);
	if(NULL == newBuf->memSpace)
	{
		fprintf(stderr, "failed to allocate memory buffer space\n");
		exit(1);
	}

	return newBuf;
}

void destroyMemoryBuffer(MemoryBuffer m)
{
	if(NULL == m)
		return;

	free(m->occupied);
	free(m->memSpace);
	free(m);
}

void* allocMemoryBufferSlot(MemoryBuffer m)
{
	if(NULL == m)
		return NULL;

	// find an empty location
	size_t startingSlot = m->currentSlot;
	do
	{
		if(!m->occupied[m->currentSlot])
		{
			// make sure the two aren't equal for later
			startingSlot = m->currentSlot + 1;
			break;
		}
		m->currentSlot = (m->currentSlot + 1) % m->numSlots;
	} while(m->currentSlot != startingSlot);

	// if no space available
	if(m->currentSlot == startingSlot)
		return NULL;

	// claim this space
	m->occupied[m->currentSlot] = 1;

	// get the actual location
	void* selected = m->memSpace + m->currentSlot * m->slotSize;

	// make next slot current
	m->currentSlot = (m->currentSlot + 1) % m->numSlots;

	return selected;
}

uint8_t freeMemoryBufferSlot(MemoryBuffer m, void* slot)
{
	if(NULL == m)
		return 0;

	// return false if this isn't a valid slot (allocated by sys?)
	int correspondingSlot;
	if(slot < m->memSpace || slot >= m->memSpace + m->numSlots * m->slotSize || m->memSpace + (correspondingSlot = (slot - m->memSpace) / m->slotSize) * m->slotSize != slot)
		return 0;

	// mark the slot free
	m->occupied[correspondingSlot] = 0;

	// successfully freed the slot
	return 1;
}
