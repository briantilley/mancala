#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "inc/LinkedStack.h"

// unique representation of a snapshot of the game
typedef struct _GameState
{
	uint8_t* board;
	uint8_t player;
} GameState;

// used in generating serialized game tree
typedef struct _StackFrame
{
	GameState* p_state;
	uint32_t nextStartPos;
} StackFrame;

// convenience constants
// unchanged for entirety of tree generation
uint32_t g_locationsPerPlayer, g_boardSize;
uint32_t g_store0idx, g_store1idx;
uint8_t g_winningThreshold;

void parseArgs(int argc, char* argv[], uint32_t* p_locationsPerPlayer, uint8_t* p_initialTokensPerLocation)
{
	// handle bad usage
	if(3 != argc)
	{
		fprintf(stderr, "usage: <executable> <locations per player (excluding stores)> <initial # of tokens per location>\n");
		exit(1);
	}

	// get values
	*p_locationsPerPlayer = atoi(argv[1]);
	*p_initialTokensPerLocation = atoi(argv[2]);

	return;
}

// print an ASCII art representation of the game board
void printState(GameState s)
{
	int i; // counter

	printf(" ______ ______ ");
	for(i = 0; i < g_locationsPerPlayer; ++i)
		printf("______ ");

	printf("\n|      |      |");
	for(i = 0; i < g_locationsPerPlayer; ++i)
		printf("      |");

	printf("\n|      |");
	for(i = 0; i < g_locationsPerPlayer; ++i)
		printf("  %2u  |", s.board[g_boardSize - 2 - i]);
	printf("      |");

	printf("\n|  %2u  |", s.board[g_boardSize - 1]);
	for(i = 0; i < g_locationsPerPlayer; ++i)
		printf("______|");
	printf("  %2u  |", s.board[g_locationsPerPlayer]);

	printf("\n|      |      |");
	for(i = 0; i < g_locationsPerPlayer; ++i)
		printf("      |");

	printf("\n|      |");
	for(i = 0; i < g_locationsPerPlayer; ++i)
		printf("  %2u  |", s.board[i]);
	printf("      |");

	printf("\n|______|______|");
	for(i = 0; i < g_locationsPerPlayer; ++i)
		printf("______|");
	printf("\n\n");

	return;
}

// help avoid accidentally setting player to invalid value
// inline
void switchPlayer(GameState* p_state)
{ p_state->player = ((0 == p_state->player) ? 1 : 0); }

// return 0 if invalid move (empty location)
// else, modify s by taking one turn
uint8_t doTurnFromLocation(uint32_t startLocation, GameState* p_state)
{
	// determine absolute initial position on board
	uint32_t currentLocation = p_state->player * (g_locationsPerPlayer + 1) + startLocation;

	// check for invalid move
	if(currentLocation > g_boardSize || 0 == p_state->board[currentLocation])
		return 0;

	// pick up tokens from currentLocation
	uint8_t tokensInHand = p_state->board[currentLocation];
	p_state->board[currentLocation] = 0;

	// record parameters
	uint32_t playerStore, opponentStore;
	playerStore = (p_state->player == 0) ? g_store0idx : g_store1idx;
	opponentStore = (playerStore == g_store0idx) ? g_store1idx : g_store0idx;

	// step forward and drop tokens in place
	while(0 < tokensInHand)
	{
		// one step forward, stay on the board
		++currentLocation; currentLocation %= g_boardSize;

		// if not over opponent's store
		if(currentLocation != opponentStore)
		{
			// drop one token into location
			p_state->board[currentLocation]++;
			tokensInHand--;
		}
	}

	// player gets to go again if they stopped on their own store
	if(currentLocation != playerStore)
	{
		// give opponent a turn
		switchPlayer(p_state);

		// if the player stopped on a previously empty location on their side,
		// they capture opponent's tokens opposite that location
		if(p_state->board[currentLocation] == 1 &&
			currentLocation < playerStore &&
			(p_state->player == 0 || currentLocation > opponentStore))
		{
			uint32_t oppositeLocation = g_boardSize - 2 - currentLocation;
			p_state->board[playerStore] += 1 + p_state->board[oppositeLocation];
			p_state->board[oppositeLocation] = p_state->board[currentLocation] = 0;
		}
	}

	// move was valid and was performed
	return 1;
}

// checks whether the range of values in the specified array is empty
// inline
uint8_t rangeIsEmpty(uint8_t values[], uint32_t start, uint32_t end)
{
	for(int i = start; i < end; ++i)
		if(values[i]) return 0;

	return 1;
}

// return true if this state is the end of a game
// inline
uint8_t isGameOver(GameState s)
{
	// either store has more than half the tokens or either side is empty
	return s.board[g_store0idx] > g_winningThreshold ||
			s.board[g_store1idx] > g_winningThreshold ||
			rangeIsEmpty(s.board, 0, g_store0idx) ||
			rangeIsEmpty(s.board, g_store0idx + 1, g_store1idx);
}

// build a fresh board with the global parameters
GameState createInitialState()
{
	// allocate space for a new board
	GameState newState;
	newState.board = (uint8_t*)malloc(g_boardSize * sizeof(uint8_t));

	// handle memory allocation failure by exiting intentionally
	if(NULL == newState.board)
	{
		fprintf(stderr, "failed to allocate space for a new state\n");
		exit(1);
	}

	// fill the board with tokens
	memset((void*)newState.board, g_winningThreshold / g_locationsPerPlayer, g_boardSize);

	// clear the players' stores
	newState.board[g_store0idx] = newState.board[g_store1idx] = 0;

	// start with player 0
	newState.player = 0;

	return newState;
}

// return a deep carbon-copy of the given state
GameState cloneState(GameState s)
{
	// allocate space for a new board
	GameState newState;
	newState.board = (uint8_t*)malloc(g_boardSize * sizeof(uint8_t));

	// handle memory allocation failure by exiting intentionally
	if(NULL == newState.board)
	{
		fprintf(stderr, "failed to allocate space for a new state\n");
		exit(1);
	}

	// copy the old board to the new one
	memcpy((void*)newState.board, (void*)s.board, g_boardSize);

	// copy player status
	newState.player = s.player;

	return newState;
}

// avoid memory leaks by cleaning up the board
// inline
void destroyState(GameState s)
{
	free(s.board);
}

// associate a game state with a stack frame
StackFrame createStackFrame(GameState* p_state)
{
	// exception case
	if(NULL == p_state)
	{
		fprintf(stderr, "cannot create a stack frame from a NULL state\n");
		exit(1);
	}

	// initialize values
	StackFrame newFrame;
	newFrame.p_state = p_state;
	newFrame.nextStartPos = 0;

	return newFrame;
}

// cleanly get rid of the frame
void destroyStackFrame(StackFrame f)
{
	destroyState(*(f.p_state));
	free(f.p_state);
}

void printSerialDecisionTree() {
	// store all the states leading up to the one being worked on
	LinkedStack currentDecisionPath = createLinkedStack();

	// make space for a starting state
	StackFrame* p_rootFrame = (StackFrame*)malloc(sizeof(StackFrame));
	if(NULL == p_rootFrame)
	{
		fprintf(stderr, "failed to allocate a root stack frame\n");
		exit(1);
	}
	p_rootFrame->p_state = (GameState*)malloc(sizeof(GameState));
	if(NULL == p_rootFrame->p_state)
	{
		fprintf(stderr, "failed to allocate a root game state\n");
		exit(1);
	}

	// make the starting state
	*(p_rootFrame->p_state) = createInitialState();

	// print the starting state
	printState(*(p_rootFrame->p_state));

	// prime the work stack
	pushLinkedStack((void*)p_rootFrame, currentDecisionPath);

	// pre-order style generation of a serialized tree
	while(NULL != currentDecisionPath.p_top)
	{
		// guaranteed non-null
		StackFrame* p_startingFrame = popLinkedStack(currentDecisionPath);

		// new state (next turn)
		GameState* p_newState = (GameState*)malloc(sizeof(GameState));
		if(NULL == p_newState)
		{
			fprintf(stderr, "failed to create a new game state in recursive section\n");
			exit(1);
		}
		*p_newState = cloneState(*(p_startingFrame->p_state));

		// iterate until a valid move is made or none are left
		if(p_startingFrame->nextStartPos < g_locationsPerPlayer)
			while(!doTurnFromLocation(p_startingFrame->nextStartPos++, p_newState) && p_startingFrame->nextStartPos <= g_locationsPerPlayer);
		else // ensure the stack is not corrupted by this
			p_startingFrame->nextStartPos = g_locationsPerPlayer + 1;

		// if a valid move was done
		if(p_startingFrame->nextStartPos <= g_locationsPerPlayer)
		{
			// print the state
			printState(*p_newState);

			// if the new state has moves to be done
			if(!isGameOver(*p_newState))
			{
				// push the popped frame back and push the new state on
			}
			// if the previous state has more moves to be done
			else if(p_startingFrame->nextStartPos < g_locationsPerPlayer)
			{
				// push popped frame back, don't push new state
			}
			// the previous state is out of moves and the state just created is game over
			else
			{
				// destroy the popped frame, don't push new state
			}
		}
		else // destroy the popped frame (it ran out of potential moves)
		{

		}
	}
}

int main(int argc, char* argv[])
{
	// get input values
	uint8_t initialTPL;
	parseArgs(argc, argv, &g_locationsPerPlayer, &initialTPL);

	// calculate game parameters
	g_boardSize = 2 * (g_locationsPerPlayer + 1);
	g_store0idx = g_locationsPerPlayer;
	g_store1idx = g_boardSize - 1;
	g_winningThreshold = g_locationsPerPlayer * initialTPL;

	// sanity check
	GameState test = createInitialState();
	GameState clone = cloneState(test);
	printState(test);
	destroyState(test);
	printState(clone);
	destroyState(clone);

	return 0;
}