#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>

/**
 * Defines bit patterns for the two special instructions, halt and nop
 */
#define HALT_PATTERN 0x8a000000
#define NOP_PATTERN 0xd503201f

/**
 * Defines the start and end bits of op0 and the bit masks and patterns which
 * are used with op0 to identify each instruction type:
 * op0   Instruction Type
 * 100x  Data Processing (Immediate)
 * x101  Data Processing (Register)
 * x1x0  Single Data Transfer
 * 101x  Branch
 */
#define OP0_START 25
#define OP0_END 28
#define DP_IMM_MASK 14
#define DP_IMM_PATTERN 8
#define DP_REG_MASK 7
#define DP_REG_PATTERN 5
#define SDT_MASK 5
#define SDT_PATTERN 4
#define BRANCH_MASK 14
#define BRANCH_PATTERN 10

/**
 * Defines the start and end bits of sf, opc, rd fields, which are shared by all
 * data processing instructions (immediate and register)
 */
#define DP_SF_START 31
#define DP_SF_END 31
#define DP_OPC_START 29
#define DP_OPC_END 30
#define DP_RD_START 0
#define DP_RD_END 4

////////////////////////////////////////////////////////////////////////////////
// Data Processing (Immediate):

/**
 * Represents the 2 types of data processing (immediate) instruction -
 * arithmetic and wide move
 */
typedef enum {
    IMM_ARITHMETIC,
    IMM_WIDE_MOVE,
 } ImmType;

/**
 * Represents the bit layout of a data processing (immediate) instruction:
 * imm_type: The type of dp immediate instruction - arithmetic, wide move
 * sf:       Register bit width (sf = 0: 32 bits, sf = 1: 64 bits)
 * opc:      Operation code, which determines the operation to be performed
 * opi:      Determines the type of dp immediate instruction
 * operand:  Value to be saved in Rd:
 *             Arithmetic -> sh: flag - if set, shift imm12 left by 12 bits
 *                           imm12: 12-bit unsigned immediate value
 *                           rn: 1st operand register
 *             Wide Move  -> hw: encodes logical shift left by hw * 16 bits
 *                           imm16: 16-bit unsigned immediate value
 * rd:       Destination register  
 */
typedef struct {
    ImmType imm_type;
    uint8_t sf;
    uint8_t opc;
    uint8_t opi;
    union {
        struct {
            uint8_t sh;
            uint16_t imm12;
            uint8_t rn;
        } Arithmetic;
        struct {
            uint8_t hw;
            uint16_t imm16;
        } WideMove;
    } operand;
    uint8_t rd;
    } DPImmFormat;

/**
 * Defines the instruction opi, which determines whether a data processing
 * (immediate) instruction is arithmetic or wide move
 */
#define OPI_START 23
#define OPI_END 25
#define ARITHMETIC_OPI 2
#define WIDE_MOVE_OPI 5

/**
 * Defines the start bit, end bit and length of fields specific to arithmetic
 * instructions: sh, imm12, rn
 * LEFT_SHIFT_SH: sh = 1 encodes a logical left shift
 */
#define SH_START 22
#define SH_END 22
#define LEFT_SHIFT_SH 1
#define IMM12_START 10
#define IMM12_END 21
#define IMM12_LENGTH 12
#define RN_START 5
#define RN_END 9

/**
 * Defines the start bit, end bit and length of fields specific to wide move
 * instructions: hw, imm16
 */
#define HW_START 21
#define HW_END 22
#define IMM16_START 5
#define IMM16_END 20
#define IMM16_LENGTH 16

/**
 * Defines the instruction opc, which is used to determine the type of wide
 * move instruction
 * opc Mnemonic
 * 00  movn - move wide with NOT (Rd := Op)
 * 10  movz - move wide with zero (Rd := Op)
 * 11  movk - move wide with keep (Rd[shift + 15 : shift] := imm16)
 */
#define MOVN_OPC 0
#define MOVZ_OPC 2
#define MOVK_OPC 3

////////////////////////////////////////////////////////////////////////////////
// Data Processing (Register):

/**
 * Represents the 3 types of data processing (register) instruction -
 * arithmetic, logical and multiply
 */
typedef enum {
    REG_ARITHMETIC,
    REG_LOGICAL,
    REG_MULTIPLY,
} RegType;

/**
 * Represents the bit layout of a data processing (register) instruction:
 * reg_type: The type of dp register instruction - arithmetic, logical, multiply
 * sf:       Register bit width (sf = 0: 32 bits, sf = 1: 64 bits)
 * opc:      Operation code, which determines the operation to be performed
 * M:        Determines whether an instruction is a multiply instruction
 * opr:      Determines whether an instruction is arithmetic or logical:
 *             Arithmetic/logical -> shift: shift type (lsl, lsr, asr, ror)
 *             Logical            -> N: N = 1: bitwise negation
 * rm:       2nd operand register - shifted for arithmetic/logical instructions
 * operand:  Operand:
 *             Arithmetic/logical -> operand: shift amount
 *             Multiply           -> x: x = 0: multiply-add, x = 1: multiply-sub
 *                                   ra: 3rd operand register
 * rn:       1st operand register
 * rd:       Destination register  
 */
typedef struct {
    RegType reg_type;
    uint8_t sf;
    uint8_t opc;
    uint8_t M;

    union {
        uint8_t arithmetic_shift;
        struct {
            uint8_t shift;
            uint8_t N;
        } logical_opr;
    } opr;

    uint8_t rm;

    union {
        uint8_t arithmetic_logical_operand;
        struct {
            uint8_t x;
            uint8_t ra;
        } multiply_operand;
    } operand;

    uint8_t rn;
    uint8_t rd;
} DPRegFormat;

/**
 * Defines the start and end bits of data processing (register) fields: M, rm,
 * operand, rn 
 */
#define M_START 28
#define M_END 28
#define RM_START 16
#define RM_END 20
#define OPERAND_START 10
#define OPERAND_END 15
#define RN_START 5
#define RN_END 9

/**
 * Defines bits used for decoding and executing arithmetic/logical instructions
 */
#define ARITHMETIC_LOGICAL_BIT 24
#define ARITHMETIC_BIT 1
#define SHIFT_START 22
#define SHIFT_END 23
#define N_START 21
#define N_END 21
#define NEGATION_N 1
#define SET_FLAGS_OPC 3

/**
 * Defines bits used for decoding and executing multiply instructions
 */
#define MULTIPLY_M 1
#define MULTIPLY_ADD_X 0
#define MULTIPLY_SUB_X 1
#define X_START 15
#define X_END 15
#define RA_START 10
#define RA_END 14

/**
 * PSTATE Z flag is set when result == 0
 */
#define Z_CASE 0

/**
 * Represents the 8 types of logical operations 
 */
typedef enum {
    AND,
    BIC,
    ORR,
    ORN,
    EOR,
    EON,
    ANDS,
    BICS,
} LogicType;

////////////////////////////////////////////////////////////////////////////////
// Data Processing (Arithmetic):

/**
 * Represents the fields of an arithmetic data processing instruction (imm/reg)
 */
typedef struct {
    uint8_t sf;
    uint8_t opc;
    uint8_t rd;
    uint8_t rn;
    uint64_t op2;
} DPArithmeticFormat;

/**
 * Defines the 4 types of arithmetic instruction: add, adds (add & set flags),
 * sub, subs (sub & set flags)
 */
typedef enum {
    ADD,
    ADDS,
    SUB,
    SUBS,
} ArithmeticType;

////////////////////////////////////////////////////////////////////////////////
// Single Data Transfer:

/**
 * Represents the 2 types of single data transfer instruction - SDT and Load
 * Literal
 */
typedef enum {
    SDT,
    LOAD_LITERAL,
} SDTType;

/**
 * Represents the 4 types of addressing mode - Unsigned Offset, Register
 * Offset, Pre-Index and Post-Index (literal addresses are handled separately)
 */
typedef enum {
    UNSIGNED_OFFSET,
    REGISTER_OFFSET,
    PRE_INDEX,
    POST_INDEX,
} AddrMode;

/**
 * Represents the bit layout of a single data transfer instruction:
 * sdt_type:    The type of single data transfer instruction- SDT, Load Literal
 * sf:          Load size (sf = 0: 32 bits, sf = 1: 64 bits)
 * SDT:         Single Data Transfer instruction subtype:
 *                addr_mode: The instruction addressing mode 
 *                L: The type of data transfer (L = 0: store, L = 1: load)
 *                offset: Used to compute memory address - depends on addr_mode
 *                  imm12: Unsigned Offset
 *                  simm9: Pre/Post-Index
 *                  xm: Register Offset
 *                xn: Base register to add the offset to
 * LoadLiteral: Load Literal instruction subtype:
 *                simm19: PC offset
 * rt:          Target register (load memory -> rt or store rt -> memory)
 */
typedef struct {
    SDTType sdt_type;
    uint8_t sf;
    union {
        struct {
            AddrMode addr_mode;
            uint8_t L;
            union {
                uint16_t imm12;
                int16_t simm9;
                uint8_t xm;
            } offset;
            uint8_t xn;
        } SDT;

        struct {
            int32_t simm19;
        } LoadLiteral;
    };
    uint8_t rt;
} SDTFormat;

/**
 * Defines the identifier bit, which determines whether an instruction is SDT 
 * (bit = 1) or Load Literal (bit = 0)
 */
#define SINGLE_DATA_TRANSFER_IDENTIFIER_START 31
#define SINGLE_DATA_TRANSFER_IDENTIFIER_END 31
#define SDT_IDENTIFIER 1
#define LOAD_LITERAL_IDENTIFIER 0

/**
 * Defines the start and end bits of rt, sf fields, which are shared by SDT
 * and Load Literal
 */ 
#define RT_START 0 
#define RT_END 4
#define SF_START 30
#define SF_END 30

/**
 * Defines the start bit, end bit and length of L, xn, xm, simm9, imm12 fields,
 * which are specific to SDT
 */
#define L_START 22
#define L_END 22
#define LOAD_L 1
#define XN_START 5
#define XN_END 9
#define XM_START 16
#define XM_END 20
#define SIMM9_START 12
#define SIMM9_END 20
#define SIMM9_LENGTH 9
#define IMM12_START 10
#define IMM12_END 21

/**
 * Defines the start bit, end bit and length of simm19, which is specific to
 * Load Literal
 */
#define SIMM19_START 5
#define SIMM19_END 23
#define SIMM19_LENGTH 19
#define LOAD_LITERAL_OFFSET_LENGTH 21

/**
 * Defines the U, I and R bits (flags for determining the addressing mode):
 * U (bit 24): U = 1 -> Unsigned Offset
 * I (bit 11): I = 1, R = 0 -> Pre-Index, I = 0, R = 0 -> Post-Index 
 * R (bit 21): R = 1 -> Register Offset
 */
#define U_START 24
#define U_END 24
#define I_START 11
#define I_END 11
#define R_START 21
#define R_END 21
#define UNSIGNED_OFFSET_U 1
#define PRE_INDEX_I 1
#define POST_INDEX_I 0
#define REGISTER_OFFSET_R 1

////////////////////////////////////////////////////////////////////////////////
// Branch:

/**
 * Represents the 3 types of branch instruction - unconditional, register and
 * conditional
 */
typedef enum {
    UNCONDITIONAL,
    REGISTER,
    CONDITIONAL,
} BranchType;

/**
 * Represents the bit layout of a branch instruction:
 * branch_type: The type of branch instruction - uncond, register, cond
 * simm26:      26-bit signed PC offset (Unconditional Branch)
 * xn:          Register containing address to jump to (Register Branch)
 * simm19:      19-bit signed PC offset (Conditional Branch)
 * cond:        4-bit encoding of condition to be satisfied (Conditional Branch)
 */
typedef struct {
    BranchType branch_type;
    union {
        struct {
            int32_t simm26;
        } Unconditional;
        
        struct {
            uint8_t xn;
        } Register;

        struct {
            int32_t simm19;
            uint8_t cond;
        } Conditional;
    };
} BranchFormat;

/** 
 * Defines the identifier bits 30-31, which uniquely identify each of the 3
 * types of branch instruction:
 * 00 - unconditional
 * 11 - register
 * 01 - conditional
 */
#define BRANCH_IDENTIFIER_START 30
#define BRANCH_IDENTIFIER_END 31
#define UNCONDITIONAL_IDENTIFIER 0
#define REGISTER_IDENTIFIER 3
#define CONDITIONAL_IDENTIFIER 1

/** 
 * Defines the start bit, end bit and length of the branch instruction fields
 * simm26, xn, simm19 and cond
 */
#define SIMM26_START 0
#define SIMM26_END 25
#define SIMM26_LENGTH 26
#define XN_START 5
#define XN_END 9
#define SIMM19_START 5
#define SIMM19_END 23
#define SIMM19_LENGTH 19
#define COND_START 0
#define COND_END 3

/**
 * Represents the 4-bit encoding of branch condition codes
 */
#define EQ 0  // ==
#define NE 1  // !=
#define GE 10 // >=
#define LT 11 // <
#define GT 12 // >
#define LE 13 // <=
#define AL 14 // always

////////////////////////////////////////////////////////////////////////////////
// General:

/**
 * Represents 4 supported instruction types from the A64 instruction set:
 * DATA_PROCESSING_IMM:  Arithmetic/logical operations using an immediate value
 * DATA_PROCESSING_REG:  Arithmetic/logical operations using registers
 * SINGLE_DATA_TRANSFER: Read/write to memory
 * BRANCH:               Modify the Program Counter
 */
typedef enum {
    DATA_PROCESSING_IMM,
    DATA_PROCESSING_REG,
    SINGLE_DATA_TRANSFER,
    BRANCH,
} InstrType;

/**
 * Represents the 4 possible formats of an instruction, depending on its type
 */
typedef union {
    DPImmFormat dp_imm_format;
    DPRegFormat dp_reg_format;
    SDTFormat sdt_format;
    BranchFormat branch_format;
} InstrFormat;

/**
 * Internally represents a decoded instruction using its type and format 
 */
typedef struct {
    InstrType type;
    InstrFormat format;
} Instr;

#endif