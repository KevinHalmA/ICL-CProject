#ifndef DP_ARITHMETIC_H
#define DP_ARITHMETIC_H

#include <stdint.h>

#include "../common/utilities.h"
#include "../common/instructions.h"

/**
 * Executes a decoded data processing arithmetic (imm/reg) instruction using a
 * pointer to its internal representation and updates the CPU state accordingly
 * opc Instruction
 * 00  add
 * 01  adds
 * 10  sub
 * 11  subs
 */
extern void execute_dp_arithmetic(DPArithmeticFormat *, CPUState *);

#endif