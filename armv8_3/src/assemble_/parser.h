#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "tokeniser.h"
#include "symbol_table.h"
#include "../common/instructions.h"

extern int parse(TokenisedString *, SymbolTable *, uint32_t, Instr *);

#endif
