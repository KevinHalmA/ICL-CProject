#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include "../common/instructions.h"

/**
 * Encodes an internal representation of an instruction as a 32-bit binary word
 */
extern uint32_t encode(Instr *);

#endif