#ifndef BRANCH_H
#define BRANCH_H

#include <stdint.h>

#include "../common/utilities.h"
#include "../common/instructions.h"

/**
 * Decodes a 32-bit branch instruction into an internal representation
 */
extern void decode_branch(uint32_t, Instr *);

/**
 * Executes a decoded branch instruction using a pointer to its internal
 * representation and updates the CPU state accordingly
 */
extern void execute_branch(Instr *, CPUState *);

#endif