#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "inc/LinkedStack.h"
#include "inc/MemoryBuffer.h"

// #define PRINT_STATES // actually print the game states
#define OS_ALLOCATION_ONLY // don't use MemoryBuffers
#define MEMORY_BUFFER_SLOT_COUNT 1024

// unique representation of a snapshot of the game
struct _GameState
{
	uint8_t* board;
	uint8_t player;
};
typedef struct _GameState* GameState;

// used in generating serialized game tree
struct _StackFrame
{
	GameState state;
	uint32_t nextStartPos;
};
typedef struct _StackFrame* StackFrame;

// convenience constants
// unchanged for entirety of tree generation
uint32_t g_locationsPerPlayer, g_boardSize;
uint32_t g_store0idx, g_store1idx;
uint8_t g_winningThreshold;

// memory buffers
MemoryBuffer g_stateBuffer, g_boardBuffer, g_stackFrameBuffer;
extern MemoryBuffer g_stackBuffer, g_nodeBuffer;

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
		printf("  %2u  |", s->board[g_boardSize - 2 - i]);
	printf("      |");
	if(s->player)
		printf(" 1");

	printf("\n|  %2u  |", s->board[g_boardSize - 1]);
	for(i = 0; i < g_locationsPerPlayer; ++i)
		printf("______|");
	printf("  %2u  |", s->board[g_locationsPerPlayer]);

	printf("\n|      |      |");
	for(i = 0; i < g_locationsPerPlayer; ++i)
		printf("      |");

	printf("\n|      |");
	for(i = 0; i < g_locationsPerPlayer; ++i)
		printf("  %2u  |", s->board[i]);
	printf("      |");
	if(!s->player)
		printf(" 0");

	printf("\n|______|______|");
	for(i = 0; i < g_locationsPerPlayer; ++i)
		printf("______|");
	printf("\n\n");

	return;
}

// help avoid accidentally setting player to invalid value
inline
void switchPlayer(GameState s)
{ s->player = ((0 == s->player) ? 1 : 0); }

// return 0 if invalid move (empty location)
// else, modify s by taking one turn
uint8_t doTurnFromLocation(uint32_t startLocation, GameState s)
{
	// determine absolute initial position on board
	uint32_t currentLocation = s->player * (g_locationsPerPlayer + 1) + startLocation;

	// check for invalid move
	if(currentLocation > g_boardSize || 0 == s->board[currentLocation])
		return 0;

	// pick up tokens from currentLocation
	uint8_t tokensInHand = s->board[currentLocation];
	s->board[currentLocation] = 0;

	// record parameters
	uint32_t playerStore, opponentStore;
	playerStore = (s->player == 0) ? g_store0idx : g_store1idx;
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
			s->board[currentLocation]++;
			tokensInHand--;
		}
	}

	// player gets to go again if they stopped on their own store
	if(currentLocation != playerStore)
	{
		// if the player stopped on a previously empty location on their side,
		// they capture opponent's tokens opposite that location
		if(s->board[currentLocation] == 1 &&
			currentLocation < playerStore &&
			(s->player == 0 || currentLocation > opponentStore))
		{
			uint32_t oppositeLocation = g_boardSize - 2 - currentLocation;
			s->board[playerStore] += 1 + s->board[oppositeLocation];
			s->board[oppositeLocation] = s->board[currentLocation] = 0;
		}

		// give opponent a turn
		switchPlayer(s);
	}

	// move was valid and was performed
	return 1;
}

// checks whether the range of values in the specified array is empty
inline
uint8_t rangeIsEmpty(uint8_t values[], uint32_t start, uint32_t end)
{
	for(int i = start; i < end; ++i)
		if(values[i]) return 0;

	return 1;
}

// return true if this state is the end of a game
inline
uint8_t isGameOver(GameState s)
{
	// either store has more than half the tokens or either side is empty
	return s->board[g_store0idx] > g_winningThreshold ||
			s->board[g_store1idx] > g_winningThreshold ||
			rangeIsEmpty(s->board, 0, g_store0idx) ||
			rangeIsEmpty(s->board, g_store0idx + 1, g_store1idx);
}

// build a blank board with the global parameters
GameState createState()
{
	// allocate space for a new state and its board
	GameState newState;
	if(NULL == (newState = allocMemoryBufferSlot(g_stateBuffer)))
		newState = (GameState)malloc(sizeof(struct _GameState));
	if(NULL == newState)
	{
		fprintf(stderr, "failed to allocate space for a new state\n");
		exit(1);
	}

	// handle memory allocation failure by exiting intentionally
	if(NULL == (newState->board = allocMemoryBufferSlot(g_boardBuffer)))
		newState->board = (uint8_t*)malloc(g_boardSize * sizeof(uint8_t));
	if(NULL == newState->board)
	{
		fprintf(stderr, "failed to allocate space for a new board\n");
		exit(1);
	}

	return newState;
}

// fill a blank board using global params
GameState createInitialState()
{
	GameState iState = createState();

	// fill the board with tokens
	memset((void*)iState->board, g_winningThreshold / g_locationsPerPlayer, g_boardSize);

	// clear the players' stores
	iState->board[g_store0idx] = iState->board[g_store1idx] = 0;

	// start with player 0
	iState->player = 0;

	return iState;
}

// return a deep carbon-copy of the given state
GameState cloneState(GameState s)
{
	// allocate space for a new board
	GameState newState = createState();

	// copy the old board to the new one
	memcpy((void*)newState->board, (void*)s->board, g_boardSize);

	// copy player status
	newState->player = s->player;

	return newState;
}

// avoid memory leaks by cleaning up the board
inline
void destroyState(GameState s)
{
	if(!freeMemoryBufferSlot(g_boardBuffer, s->board))
		free(s->board);
	if(!freeMemoryBufferSlot(g_stateBuffer, s))
		free(s);
}

// associate a game state with a stack frame
StackFrame createStackFrame(GameState s)
{
	// exception case
	if(NULL == s)
	{
		fprintf(stderr, "cannot create a stack frame from a NULL state\n");
		exit(1);
	}

	// allocate space for new frame
	StackFrame newFrame;
	if(NULL == (newFrame = allocMemoryBufferSlot(g_stackFrameBuffer)))
		newFrame = (StackFrame)malloc(sizeof(struct _StackFrame));
	if(NULL == newFrame)
	{
		fprintf(stderr, "failed to create a new stack frame\n");
		exit(1);
	}

	// initialize values
	newFrame->state = s;
	newFrame->nextStartPos = 0;

	return newFrame;
}

// cleanly get rid of the frame
inline
void destroyStackFrame(StackFrame f)
{
	destroyState(f->state);
	if(!freeMemoryBufferSlot(g_stackFrameBuffer, f))
		free(f);
}

// return true if there are remaining moves for this frame
inline
uint8_t hasRemainingMoves(StackFrame f)
{
	uint32_t start = f->state->player * (g_locationsPerPlayer + 1) + f->nextStartPos;
	uint32_t end = (f->state->player == 0) ? g_store0idx : g_store1idx;
	return !rangeIsEmpty(f->state->board, start, end);
}

void printSerialDecisionTree() {
	// store all the states leading up to the one being worked on
	LinkedStack currentDecisionPath = createLinkedStack();

	// make the starting state
	GameState freshState = createInitialState();

	// wrap the state in a stack frame
	StackFrame startingFrame = createStackFrame(freshState);

#ifdef PRINT_STATES
	// print the starting state
	printState(freshState);
#endif

	// pre-order style generation of a serialized tree
	do
	{
		// new state (next turn)
		GameState newState = cloneState(startingFrame->state);

		// iterate until a valid move is made or none are left
		uint8_t turnWasValid = 0;
		while(startingFrame->nextStartPos < g_locationsPerPlayer && !(turnWasValid = doTurnFromLocation(startingFrame->nextStartPos++, newState)));

		// if a valid move was done
		if(turnWasValid)
		{

#ifdef PRINT_STATES
			// print the state
			printState(newState);
#endif

			// if the new state has moves to be done
			if(!isGameOver(newState))
			{
				// push the frame we started at, hold the new state
				pushLinkedStack(startingFrame, currentDecisionPath);
				startingFrame = createStackFrame(newState);
			}
			// if the starting frame has more moves to be done and game ends at new state
			// else if(startingFrame->nextStartPos < g_locationsPerPlayer)
			else if(hasRemainingMoves(startingFrame))
			{
				// keep holding popped frame, eliminate new state
				destroyState(newState);
			}
			// the previous state is out of moves and the state just created is game over
			else
			{
				// destroy the processed frame, don't push new state
				destroyStackFrame(startingFrame);
				destroyState(newState);

				// get one frame up for the go-around
				startingFrame = popLinkedStack(currentDecisionPath);
			}
		}
		else // destroy the processed frame (it ran out of potential moves)
		{
			destroyStackFrame(startingFrame);
			destroyState(newState);

			// get one frame up for the go-around
			startingFrame = popLinkedStack(currentDecisionPath);
		}
	} while(NULL != startingFrame);
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

#ifndef OS_ALLOCATION_ONLY
	// block out sizable memory buffers
	g_stateBuffer = (MemoryBuffer)createMemoryBuffer(MEMORY_BUFFER_SLOT_COUNT, sizeof(struct _GameState));
	g_boardBuffer = (MemoryBuffer)createMemoryBuffer(MEMORY_BUFFER_SLOT_COUNT, g_boardSize * sizeof(uint8_t));
	g_stackFrameBuffer = (MemoryBuffer)createMemoryBuffer(MEMORY_BUFFER_SLOT_COUNT, sizeof(struct _StackFrame));
	g_stackBuffer = (MemoryBuffer)createMemoryBuffer(MEMORY_BUFFER_SLOT_COUNT, sizeof(struct _LinkedStack));
	g_nodeBuffer = (MemoryBuffer)createMemoryBuffer(MEMORY_BUFFER_SLOT_COUNT, sizeof(struct _LinkedStackNode));
#else
	g_stateBuffer = g_boardBuffer = g_stackFrameBuffer = g_stackBuffer = g_nodeBuffer = NULL;
#endif

	// // sanity check
	// GameState test = createInitialState();
	// GameState clone = cloneState(test);
	// printState(test);
	// destroyState(test);
	// printState(clone);
	// destroyState(clone);

	printSerialDecisionTree();

	return 0;
}