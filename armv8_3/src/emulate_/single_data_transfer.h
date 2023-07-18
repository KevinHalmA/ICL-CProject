#ifndef SINGLE_DATA_TRANSFER_H
#define SINGLE_DATA_TRANSFER_H

#include "../common/utilities.h"
#include "../common/instructions.h"

/**
 * Decodes a 32-bit single data transfer instruction into an internal
 * representation
 */
extern void decode_single_data_transfer(uint32_t, Instr *);

/**
 * Executes a decoded single data transfer instruction using a pointer to its
 * internal representation and updates the CPU state accordingly
 */
extern void execute_single_data_transfer(Instr *, CPUState *);

#endif