#ifndef LINKED_STACK_H
#define LINKED_STACK_H

typedef struct _LinkedStackNode
{
	void* p_data;
	struct _LinkedStackNode* p_next;
} LinkedStackNode;

typedef struct _LinkedStack
{
	LinkedStackNode* p_top;
} LinkedStack;

// safely make a new stack
LinkedStack createLinkedStack();

// safely delete the entire stack
void destroyLinkedStack(LinkedStack s);

void* popLinkedStack(LinkedStack s);

void pushLinkedStack(void* p_data, LinkedStack s);

#endif