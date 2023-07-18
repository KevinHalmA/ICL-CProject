#ifndef EMULATOR_H
#define EMULATOR_H

#include <stdio.h>

#include "../common/utilities.h"

/**
 * Initialises the CPU state:
 * Sets memory locations and general-purpose register values to 0, PC = 0x0,
 * ZR = 0, and PSTATE condition flags {N, Z, C, F} = {0, 1, 0, 0}
 */
extern int initialise_emulator(CPUState *);

/**
 * Runs the main execution pipeline of the emulator:
 * Until the halt instruction is reached, repeatedly fetches the next
 * instruction from memory, decodes it and executes it, updating the CPU state
 */
extern void run_emulator(CPUState *);

/**
 * Writes the CPU state (general-purpose registers, program counter, PSTATE
 * condition flags, non-zero memory) to a file stream specified by a pointer
 */
extern void write_output(CPUState *, FILE *);

/**
 * Frees all dynamically allocated memory associated with the CPU state
 */
extern void free_emulator(CPUState *);

#endif