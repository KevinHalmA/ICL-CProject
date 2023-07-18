#ifndef MNEMONICS_H
#define MNEMONICS_H

// Characters for 32-bit mode and 64-bit mode register access respectively
#define CHAR_32_BIT 'w'
#define CHAR_64_BIT 'x'

// Mnemonic for the zero register
#define ZERO_REG_STR "zr"

// Mnemonics for special instructions
#define NOP_STR "nop"

// Mnemonics for data processing (arithmetic) instructions
#define DP_ADD_STR "add"
#define DP_ADDS_STR "adds"
#define DP_SUB_STR "sub"
#define DP_SUBS_STR "subs"
#define DP_CMP_STR "cmp"
#define DP_CMN_STR "cmn"
#define DP_NEG_STR "neg"
#define DP_NEGS_STR "negs"

// Mnemonics for data processing (logical) instructions
#define DP_AND_STR "and"
#define DP_ANDS_STR "ands"
#define DP_BIC_STR "bic"
#define DP_BICS_STR "bics"
#define DP_EOR_STR "eor"
#define DP_EON_STR "eon"
#define DP_ORR_STR "orr"
#define DP_ORN_STR "orn"
#define DP_TST_STR "tst"
#define DP_MVN_STR "mvn"
#define DP_MOV_STR "mov"

// Mnemonics for data processing (wide move) instructions
#define DP_MOVN_STR "movn"
#define DP_MOVK_STR "movk"
#define DP_MOVZ_STR "movz"

// Mnemonics for data processing (multiply) instructions
#define DP_MADD_STR "madd"
#define DP_MSUB_STR "msub"
#define DP_MUL_STR "mul"
#define DP_MNEG_STR "mneg"

// Mnemonics for single data transfer instructions
#define SDT_LDR_STR "ldr"
#define SDT_STR_STR "str"

// Mnemonics for branch instructions
#define BRANCH_STR "b"
#define BRANCH_REG_STR "br"

// Mnemonics for types of arithmetic/logical shifts
#define LSL_STR "lsl"
#define LSR_STR "lsr"
#define ASR_STR "asr"
#define ROR_STR "ror"

// Mnemonics for types of branch conditions
#define EQ_STR "eq"
#define NE_STR "ne"
#define GE_STR "ge"
#define LT_STR "lt"
#define GT_STR "gt"
#define LE_STR "le"
#define AL_STR "al"

#endif