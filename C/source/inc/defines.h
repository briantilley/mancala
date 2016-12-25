#ifndef DEFINES_H
#define DEFINES_H

#define PRINT_STATES // actually print the game states
#define COLLECT_GAME_METRICS
// #define OS_ALLOCATION_ONLY // don't use MemoryBuffers
#define MEMORY_BUFFER_SLOT_COUNT 64

#ifndef OS_ALLOCATION_ONLY
#define ASSUME_MEMORY_BUFFER_SUCCESS
#endif

#endif