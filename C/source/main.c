#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// unique representation of a snapshot of the game
typedef struct _GameState
{
	uint8_t* board;
	uint8_t player;
} GameState;

// used in generating serialized game tree
typedef struct _StackFrame
{
	GameState state;
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

// build a fresh board with the global parameters
GameState createInitialState()
{
	// allocate space for a new board
	GameState newState;
	newState.board = (uint8_t*)malloc(g_boardSize * sizeof(uint8_t));

	if(NULL == newState.board)
	{
		fprintf(stderr, "failed to allocate space for a new state\n");
		exit(1);
	}

	// fill the board with tokens
	memset((void*)newState.board, g_winningThreshold / g_locationsPerPlayer, g_boardSize);
	// for(int i = 0; i < g_boardSize; ++i)
	// 	newState.board[i] = g_winningThreshold / g_locationsPerPlayer;

	// clear the players' stores
	newState.board[g_store0idx] = newState.board[g_store1idx] = 0;

	// start with player 0
	newState.player = 0;

	return newState;
}

// avoid memory leaks
GameState destroyState(GameState s)
{
	free(s.board);
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
	printState(test);
	destroyState(test);

	return 0;
}