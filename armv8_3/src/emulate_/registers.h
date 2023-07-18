#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

#include "../common/utilities.h"

/**
 * Reads the value of a register, either in 64-bit mode or 32-bit mode
 * Pre: Register index is in valid range (0 to 31)
 */
extern uint64_t read_register(BitMode, uint64_t *, uint32_t);

/**
 * Writes a value to a register, either in 64-bit mode or 32-bit mode
 * Pre: Register index is in valid range (0 to 31)
 */
extern void write_register(BitMode, uint64_t *, uint32_t, uint64_t);

#endif