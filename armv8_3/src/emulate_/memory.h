#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#include "../common/utilities.h"

/**
 * Reads a value stored at an address in little-endian memory, either in 32-bit
 * or 64-bit mode
 * Pre: Memory address is in valid range
 */
extern uint64_t read_memory(BitMode, uint8_t *, uint64_t);

/**
 * Writes a value to an address in little-endian memory, either in 32-bit or
 * 64-bit mode
 * Pre: Memory address is in valid range
 */
extern void write_memory(BitMode, uint8_t *, uint64_t, uint64_t);

#endif