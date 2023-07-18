#ifndef DP_REGISTER_H
#define DP_REGISTER_H

#include <stdint.h>

#include "../common/utilities.h"
#include "../common/instructions.h"

/**
 * Decodes a 32-bit data processing (register) instruction into an internal
 * representation
 */
extern void decode_dp_reg(uint32_t, Instr *);

/**
 * Executes a decoded data processing (register) instruction using a pointer to
 * its internal representation and updates the CPU state accordingly
 */
extern void execute_dp_reg(Instr *, CPUState *);

#endif