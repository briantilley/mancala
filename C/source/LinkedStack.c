#include <stdio.h>
#include <stdlib.h>

#include "inc/LinkedStack.h"
#include "inc/MemoryBuffer.h"
#include "inc/defines.h"

// memory buffers
MemoryBuffer g_stackBuffer = NULL, g_nodeBuffer = NULL;

// safely make a new stack
LinkedStack createLinkedStack()
{
	LinkedStack newStack;
	#ifdef ASSUME_MEMORY_BUFFER_SUCCESS
	newStack = allocMemoryBufferSlot(g_stackBuffer);
	#else
	#ifndef OS_ALLOCATION_ONLY
	if(NULL == (newStack = allocMemoryBufferSlot(g_stackBuffer)))
	#endif
		newStack = (LinkedStack)malloc(sizeof(struct _LinkedStack));
	#endif
	if(NULL == newStack)
	{
		fprintf(stderr, "failed to create a new LinkedStack\n");
		exit(1);
	}

	// make sure stack starts empty
	newStack->top = NULL;

	return newStack;
}

// safely delete the entire stack
void destroyLinkedStack(LinkedStack s)
{
	if(NULL != s->top)
	{
		fprintf(stderr, "attempted to destroy a non-empty stack\n");
		exit(1);
	}

	#ifdef ASSUME_MEMORY_BUFFER_SUCCESS
	freeMemoryBufferSlot(g_stackBuffer, s);
	#else
	#ifndef OS_ALLOCATION_ONLY
	if(!freeMemoryBufferSlot(g_stackBuffer, s))
	#endif
		free(s);
	#endif
}

LinkedStackNode createLinkedStackNode()
{
	// allocate the node
	LinkedStackNode newNode;
	#ifdef ASSUME_MEMORY_BUFFER_SUCCESS
	newNode = allocMemoryBufferSlot(g_nodeBuffer);
	#else
	#ifndef OS_ALLOCATION_ONLY
	if(NULL == (newNode = allocMemoryBufferSlot(g_nodeBuffer)))
	#endif
		newNode = (LinkedStackNode)malloc(sizeof(struct _LinkedStackNode));
	#endif
	if(NULL == newNode)
	{
		fprintf(stderr, "failed to create new LinkedStackNode\n");
		exit(1);
	}

	// initialize members
	newNode->p_data = NULL;
	newNode->next = NULL;

	return newNode;
}

void destroyLinkedStackNode(LinkedStackNode n)
{
	// assume data and next node are still in use
	#ifdef ASSUME_MEMORY_BUFFER_SUCCESS
	freeMemoryBufferSlot(g_nodeBuffer, n);
	#else
	#ifndef OS_ALLOCATION_ONLY
	if(!freeMemoryBufferSlot(g_nodeBuffer, n))
	#endif
		free(n);
	#endif
}

void* popLinkedStack(LinkedStack s)
{
	// handle empty stack
	if(NULL == s->top)
		return NULL;

	// grab pointer to return
	void* p_data = s->top->p_data;

	// unlink top node
	LinkedStackNode popped = s->top;
	s->top = s->top->next;

	// clean up
	destroyLinkedStackNode(popped);

	return p_data;
}

void pushLinkedStack(void* p_data, LinkedStack s)
{
	// create a stack node
	LinkedStackNode newNode = createLinkedStackNode();

	// fill in new node
	newNode->p_data = p_data;
	newNode->next = s->top;

	// set new top node for s
	s->top = newNode;
}