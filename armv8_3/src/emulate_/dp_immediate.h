#ifndef DP_IMMEDIATE_H
#define DP_IMMEDIATE_H

#include <stdint.h>

#include "../common/utilities.h"
#include "../common/instructions.h"

/**
 * Decodes a 32-bit data processing (immediate) instruction into an internal
 * representation
 */
extern void decode_dp_imm(uint32_t, Instr *);

/**
 * Executes a decoded data processing (immediate) instruction using a pointer to
 * its internal representation and updates the CPU state accordingly
 */
extern void execute_dp_imm(Instr *, CPUState *);

#endif