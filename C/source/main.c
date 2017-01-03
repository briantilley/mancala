#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "inc/defines.h"

// unique representation of a snapshot of the game
struct _GameState
{
	uint8_t* board;
	uint8_t player;
	uint32_t nextStartPos;
};
typedef struct _GameState GameState;

// convenience constants
// unchanged for entirety of tree generation
uint32_t g_locationsPerPlayer, g_boardSize;
uint32_t g_store0idx, g_store1idx;
uint8_t g_winningThreshold;

// stack storage for states
GameState g_stateStack[STATE_STACK_SIZE];
int32_t g_stateStackNext = 0;

// convenience
#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

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
	unsigned i; // counter

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
	if(s.player)
		printf(" 1");

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
	if(!s.player)
		printf(" 0");

	printf("\n|______|______|");
	for(i = 0; i < g_locationsPerPlayer; ++i)
		printf("______|");
	printf("\n\n");

	return;
}

// help avoid accidentally setting player to invalid value
void switchPlayer(GameState* p_state)
{ p_state->player = ((0 == p_state->player) ? 1 : 0); }

// return 0 if invalid move (empty location)
// else, modify s by taking one turn
uint8_t doTurn(GameState* p_state)
{
	// determine absolute initial position on board
	uint32_t currentLocation = p_state->player * (g_locationsPerPlayer + 1) + p_state->nextStartPos;

	// reset the start position for the newly-made state
	p_state->nextStartPos = 0;

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

		// give opponent a turn
		switchPlayer(p_state);
	}

	// move was valid and was performed
	return 1;
}

// checks whether the range of values in the specified array is empty
uint8_t rangeIsEmpty(uint8_t values[], uint32_t start, uint32_t end)
{
	for(unsigned i = start; i < end; ++i)
		if(values[i]) return 0;

	return 1;
}

// return true if this state is the end of a game
uint8_t isGameOver(GameState s)
{
	// either store has more than half the tokens or either side is empty
	return s.board[g_store0idx] > g_winningThreshold ||
			s.board[g_store1idx] > g_winningThreshold ||
			rangeIsEmpty(s.board, 0, g_store0idx) ||
			rangeIsEmpty(s.board, g_store0idx + 1, g_store1idx);
}

// fill a blank board using global params
void initializeState(GameState* p_state)
{
	// fill the board with tokens
	memset((void*)p_state->board, g_winningThreshold / g_locationsPerPlayer, g_boardSize);

	// clear the players' stores
	p_state->board[g_store0idx] = p_state->board[g_store1idx] = 0;

	// start with player 0
	p_state->player = 0;

	// start at move 0
	p_state->nextStartPos = 0;
}

// copy a state
void cloneState(GameState* p_copy, GameState* p_source)
{
	// copy the old board to the new one
	memcpy((void*)p_copy->board, (void*)p_source->board, g_boardSize);

	// copy player status
	p_copy->player = p_source->player;

	// copy next location from which to move
	p_copy->nextStartPos = p_source->nextStartPos;
}

// steps one too far if no valid move is found
// will always start with the position specified by f
void stepToNextValidMove(GameState* p_state)
{
	// check the active player's side, repeatedly incrementing nextStartPos
	uint32_t offset = p_state->player * (g_locationsPerPlayer + 1);
	do
	{
		if(p_state->board[offset + p_state->nextStartPos])
			break;
	} while(++p_state->nextStartPos < g_locationsPerPlayer);
}

void printSerialDecisionTree() {
	// game metrics
	#ifdef COLLECT_GAME_METRICS
	uint64_t gameCount = 0, moveCount = 0;
	#endif

	// get two adjacent states from the stack
	int32_t beforeStateIdx = g_stateStackNext++;
	int32_t afterStateIdx = g_stateStackNext++;

	// initialize
	initializeState(g_stateStack + beforeStateIdx);

	#ifdef PRINT_STATES
	// print the starting state
	printState(g_stateStack[beforeStateIdx]);
	#endif

	// pre-order style generation of a serialized tree
	while(0 <= beforeStateIdx)
	{
		// start at a good move if the one chosen is not already
		stepToNextValidMove(g_stateStack + beforeStateIdx);

		// if there is no valid move to make
		if(g_stateStack[beforeStateIdx].nextStartPos >= g_locationsPerPlayer)
		{
			// jump up the stack to find a state with moves left
			afterStateIdx = beforeStateIdx--;

			if(beforeStateIdx < 2)
			{
				if(beforeStateIdx > 0)
					printf(" ");
				printf("%d\n", g_stateStack[beforeStateIdx].nextStartPos);
			}

			// try again
			continue;
		}

		// make a copy to modify
		cloneState(g_stateStack + afterStateIdx, g_stateStack + beforeStateIdx);

		// ensure the copied state starts at a new move next time around
		g_stateStack[beforeStateIdx].nextStartPos++;

		// we know we're at a valid move, do it
		doTurn(g_stateStack + afterStateIdx);

		#ifdef PRINT_STATES
		// print the state found
		printState(g_stateStack[afterStateIdx]);
		#endif

		#ifdef COLLECT_GAME_METRICS
		moveCount++;
		#endif

		// if the new state has moves to be done
		if(!isGameOver(g_stateStack[afterStateIdx]))
		{
			// if there were still more moves, leave the after state on the stack
			beforeStateIdx = afterStateIdx++;
		}
		#ifdef COLLECT_GAME_METRICS
		else
			gameCount++;
		#endif
	}

	#ifdef COLLECT_GAME_METRICS
	printf("  total games: %lu\n total states: %lu\n",
		gameCount, moveCount + 1);
	#endif
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

	// allocate the board for all stack slots
	uint8_t* allBoards = (uint8_t*)malloc(STATE_STACK_SIZE * g_boardSize * sizeof(uint8_t));
	if(NULL == allBoards)
	{
		fprintf(stderr, "failed to allocate boards\n");
		exit(1);
	}

	// assign pointers within the full allocation
	for(int i = 0; i < STATE_STACK_SIZE; ++i)
	{
		g_stateStack[i].board = allBoards + i * g_boardSize;
	}

	printSerialDecisionTree();

	// deallocate the boards
	free(allBoards);

	return 0;
}