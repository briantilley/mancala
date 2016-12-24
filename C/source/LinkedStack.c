#include <stdio.h>
#include <stdlib.h>

#include "inc/LinkedStack.h"

// safely make a new stack
LinkedStack createLinkedStack()
{
	LinkedStack newStack;

	// make sure stack starts empty
	newStack.p_top = NULL;

	return newStack;
}

// safely delete the entire stack
void destroyLinkedStack(LinkedStack s)
{

}

void* popLinkedStack(LinkedStack s)
{
	// handle empty stack
	if(NULL == s.p_top)
		return NULL;

	// grab pointer to return
	void* p_data = s.p_top->p_data;

	// unlink top node
	LinkedStackNode* p_popped = s.p_top;
	s.p_top = s.p_top->p_next;

	// deallocate memory
	free(p_popped);

	return p_data;
}

void pushLinkedStack(void* p_data, LinkedStack s)
{
	// create a stack node and attempt allocation
	LinkedStackNode* p_new = (LinkedStackNode*)malloc(sizeof(LinkedStackNode));

	// check for failed allocation
	if(NULL == p_new)
	{
		fprintf(stderr, "failed to allocate space for a new stack node, exiting\n");
		exit(1);
	}

	// fill in new node
	p_new->p_data = p_data;
	p_new->p_next = s.p_top;

	// set new top node for s
	s.p_top = p_new;
}