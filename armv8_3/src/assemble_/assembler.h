#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "symbol_table.h"

/**
 * Performs the 1st pass of the assembler over the source code
 * Reads assembly file line by line and adds all labels found to a given
 * symbol table, along with their respective memory addresses
 */
extern void first_pass(FILE *, SymbolTable *);

/**
 * Performs the 2nd pass of the assembler over the source code
 * Reads assembly file line by line and processes all .int directives and
 * instructions
 * Tokenises, parses and encodes each instruction and writes the result to a
 * given binary file 
 * Returns -1 for failure, 0 for success
 */
extern int second_pass(FILE *, FILE *, SymbolTable *);

#endif