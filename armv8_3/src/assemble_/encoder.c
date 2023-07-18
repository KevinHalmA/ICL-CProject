#include <stdint.h>
#include <assert.h>

#include "encoder.h"
#include "../common/utilities.h"
#include "../common/instructions.h"

// Represents the base mask and shift shared by all dp immediate instructions 
#define DP_IMM_BASE 1
#define DP_IMM_BASE_SHIFT 28

// Represents the base mask and shift shared by all dp register instructions 
#define DP_REG_BASE 5
#define DP_REG_BASE_SHIFT 25
// Bit masks for multiply and arithmetic instructions
#define MULTIPLY_MASK 0x11000000
#define ARITHMETIC_MASK 0x1000000

// Represents the base mask and shift shared by all single data transfer instructions 
#define SINGLE_DATA_TRANSFER_BASE 3
#define SINGLE_DATA_TRANSFER_BASE_SHIFT 27
// Bit masks for sdt, register offset and pre/post index instructions
#define SDT_TYPE_MASK 0xa0000000
#define REGISTER_OFFSET_MASK 0x206800
#define PRE_POST_INDEX_MASK 0x400

// Represents the base mask and shift shared by all branch instructions 
#define BRANCH_BASE 5
#define BRANCH_BASE_SHIFT 26
// Bit mask for branch register instructions
#define REGISTER_MASK 0x21f0000

/**
 * Encodes an internal representation of a data processing (immediate)
 * instruction as a 32-bit binary word
 */
static uint32_t encode_dp_imm(Instr *);

/**
 * Encodes an internal representation of a data processing (register)
 * instruction as a 32-bit binary word
 */
static uint32_t encode_dp_reg(Instr *);

/**
 * Encodes an internal representation of a single data transfer instruction as a
 * 32-bit binary word
 */
static uint32_t encode_single_data_transfer(Instr *);

/**
 * Encodes an internal representation of a branch instruction as a 32-bit binary
 * word
 */
static uint32_t encode_branch(Instr *instr);

/**
 * Declares a type EncodePtr representing a pointer to an encode function
 */ 
typedef uint32_t (*EncodePtr)(Instr *);

/**
 * Declares a key-value pair with key = instruction type, value = pointer to an
 * encode function
 */
typedef struct {
    InstrType type;
    EncodePtr func_ptr;
} EncodeEntry;

/**
 * Defines a table (array of structs) that maps the instruction type of the
 * internal representation to a pointer to its corresponding encode function
 */
static EncodeEntry encodeTable[] = {
    {DATA_PROCESSING_IMM, &encode_dp_imm},
    {DATA_PROCESSING_REG, &encode_dp_reg},
    {SINGLE_DATA_TRANSFER, &encode_single_data_transfer},
    {BRANCH, &encode_branch}
};

uint32_t encode(Instr *instr) {
    // Loops through the encodeTable
    for (int i = 0; i < sizeof(encodeTable) / sizeof(encodeTable[0]); i++) {
        // Delegates the encoding to the appropriate function for the instruction type
        if (encodeTable[i].type == instr->type) {
            return encodeTable[i].func_ptr(instr);
        }
    }
    // Assume valid instruction - should not reach this case
    assert(0);
}

static uint32_t encode_dp_imm(Instr *instr) {
    DPImmFormat format = instr->format.dp_imm_format;

    // Encodes fields shared by all data processing immediate instructions
    uint32_t encoded = (DP_IMM_BASE << DP_IMM_BASE_SHIFT)
                     | (format.sf << DP_SF_START)
                     | (format.opc << DP_OPC_START)
                     | (format.opi << OPI_START)
                     | (format.rd << DP_RD_START);

    // Encodes fields specific to imm arithmetic or wide move instructions
    switch (format.imm_type) {
        case IMM_ARITHMETIC:
            encoded |= (format.operand.Arithmetic.sh << SH_START)
                    | (format.operand.Arithmetic.imm12 << IMM12_START)
                    | (format.operand.Arithmetic.rn << RN_START);
            break;
        case IMM_WIDE_MOVE:
            encoded |= (format.operand.WideMove.hw << HW_START)
                    | (format.operand.WideMove.imm16 << IMM16_START);
            break;
        default:
            // Assume valid instruction - should not reach this case
            assert(0);
    }

    return encoded;
}

static uint32_t encode_dp_reg(Instr *instr) {
    DPRegFormat format = instr->format.dp_reg_format;

    // Encodes fields shared by all data processing register instructions
    uint32_t encoded = (DP_REG_BASE << DP_REG_BASE_SHIFT)
                     | (format.sf << DP_SF_START)
                     | (format.rm << RM_START)
                     | (format.rn << RN_START)
                     | (format.rd << DP_RD_START);

    // Encodes fields specific to reg multiply, arithmetic or logical instructions
    if (format.reg_type == REG_MULTIPLY) {
        encoded |= MULTIPLY_MASK
                | (format.operand.multiply_operand.x << X_START)
                | (format.operand.multiply_operand.ra << RA_START);
    } else {
        encoded |= (format.opc << DP_OPC_START)
                | (format.M << M_START)
                | (format.operand.arithmetic_logical_operand << OPERAND_START);
        
        switch (format.reg_type) {
            case REG_ARITHMETIC:
                encoded |= ARITHMETIC_MASK
                        | format.opr.arithmetic_shift << SHIFT_START;
                break;
            case REG_LOGICAL:
                encoded |= (format.opr.logical_opr.shift << SHIFT_START)
                        | (format.opr.logical_opr.N << N_START);
                break;
            default:
                // Assume valid instruction - should not reach this case
                assert(0);
        }
    }

    return encoded;
}

static uint32_t encode_single_data_transfer(Instr *instr) {
    SDTFormat format = instr->format.sdt_format;

    // Encodes fields shared by all single data transfer instructions
    uint32_t encoded = (SINGLE_DATA_TRANSFER_BASE << SINGLE_DATA_TRANSFER_BASE_SHIFT)
                     | (format.sf << SF_START)
                     | (format.rt << RT_START);

    switch (format.sdt_type) {
        case SDT:
            // Encodes fields shared by all SDT instructions
            encoded |= SDT_TYPE_MASK
                    | (format.SDT.L << L_START)
                    | (format.SDT.xn << XN_START);
            // Encodes fields specific to each instruction addressing mode
            switch (format.SDT.addr_mode) {
                case UNSIGNED_OFFSET:
                    encoded |= (UNSIGNED_OFFSET_U << U_START)
                            | (format.SDT.offset.imm12 << IMM12_START);
                    break;
                case REGISTER_OFFSET:
                    encoded |= (format.SDT.offset.xm << XM_START)
                            | REGISTER_OFFSET_MASK;
                    break;
                case PRE_INDEX:
                case POST_INDEX:
                    encoded |= ((format.SDT.addr_mode == PRE_INDEX ? PRE_INDEX_I : POST_INDEX_I) << I_START)
                            | ((format.SDT.offset.simm9 << SIMM9_START) & get_bit_mask(SIMM9_START, SIMM9_END))
                            | PRE_POST_INDEX_MASK;
                    break;
                default:
                    // Assume valid instruction - should not reach this case
                    assert(0);
            }
            break;
        case LOAD_LITERAL:
            // Encodes fields shared by all Load Literal instructions
            encoded |= format.LoadLiteral.simm19 << SIMM19_START & get_bit_mask(SIMM19_START, SIMM19_END);
            break;
        default:
            // Assume valid instruction - should not reach this case
            assert(0);
    }

    return encoded;
}

static uint32_t encode_branch(Instr *instr) {
    BranchFormat format = instr->format.branch_format;
    
    uint32_t encoded = BRANCH_BASE << BRANCH_BASE_SHIFT;

    // Encodes fields specific to each branch instruction type
    switch (format.branch_type) {
        case UNCONDITIONAL:
            encoded |= (UNCONDITIONAL_IDENTIFIER << BRANCH_IDENTIFIER_START)
                    | ((format.Unconditional.simm26 << SIMM26_START) & get_bit_mask(SIMM26_START, SIMM26_END));
            break;
        case REGISTER:
            encoded |= (REGISTER_IDENTIFIER << BRANCH_IDENTIFIER_START)
                    | REGISTER_MASK
                    | (format.Register.xn << XN_START);
            break;
        case CONDITIONAL:
            encoded |= (CONDITIONAL_IDENTIFIER << BRANCH_IDENTIFIER_START)
                    | ((format.Conditional.simm19 << SIMM19_START) & get_bit_mask(SIMM19_START, SIMM19_END))
                    | (format.Conditional.cond << COND_START);
            break;
        default:
            // Assume valid instruction - should not reach this case
            assert(0);
    }

    return encoded;
}