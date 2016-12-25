#include <stdio.h>
#include <stdlib.h>

#include "inc/LinkedStack.h"
#include "inc/MemoryBuffer.h"

// memory buffers
MemoryBuffer g_stackBuffer, g_nodeBuffer;

// safely make a new stack
LinkedStack createLinkedStack()
{
	LinkedStack newStack;
	if(NULL == (newStack = allocMemoryBufferSlot(g_stackBuffer)))
		newStack = (LinkedStack)malloc(sizeof(struct _LinkedStack));
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

}

LinkedStackNode createLinkedStackNode()
{
	// allocate the node
	LinkedStackNode newNode;
	if(NULL == (newNode = allocMemoryBufferSlot(g_nodeBuffer)))
		newNode = (LinkedStackNode)malloc(sizeof(struct _LinkedStackNode));
	if(NULL == newNode)
	{
		fprintf(stderr, "failed to create new LinkedStackNode\n");
		exit(1);
	}

	// initialize members
	newNode->p_data = NULL;
	newNode->next = NULL;
}

void destroyLinkedStackNode(LinkedStackNode n)
{
	// assume data and next node are still in use
	free(n);
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