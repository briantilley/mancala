#ifndef LINKED_STACK_H
#define LINKED_STACK_H

struct _LinkedStackNode
{
	void* p_data;
	struct _LinkedStackNode* next;
};
typedef struct _LinkedStackNode* LinkedStackNode;

struct _LinkedStack
{
	LinkedStackNode top;
};
typedef struct _LinkedStack* LinkedStack;

// safely make a new stack
LinkedStack createLinkedStack();

// safely delete the entire stack
void destroyLinkedStack(LinkedStack s);

void* popLinkedStack(LinkedStack s);

void pushLinkedStack(void* p_data, LinkedStack s);

#endif