#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>

#include "parser.h"
#include "tokeniser.h"
#include "symbol_table.h"
#include "mnemonics.h"
#include "../common/utilities.h"
#include "../common/instructions.h"

// The position of the mnemonic in a list of string tokens
#define MNEMONIC_INDEX 0

// Bit shift and mask for obtaining the opc and N fields of logical instructions
#define OPC_SHIFT 1
#define N_MASK 1

/**
 * Struct representing the result of a parsed register token - holds the
 * register index (0-31) and bit mode (0 for 32-bit, 1 for 64-bit)
 */
typedef struct {
    uint32_t index;
    int bit_mode;
} ParsedRegister;

/**
 * Struct representing an alias, its corresponding mnemonic and the position
 * where the zero register string (wzr/xzr) should be inserted
 */
typedef struct {
    char *alias;
    char *mnemonic;
    int index;
} AliasEntry;

/**
 * Table of alias mappings
 */
static AliasEntry aliasTable[] = {
        {DP_CMP_STR, DP_SUBS_STR, 1},
        {DP_CMN_STR, DP_ADDS_STR, 1},
        {DP_NEG_STR, DP_SUB_STR, 2},
        {DP_NEGS_STR, DP_SUBS_STR, 2},
        {DP_TST_STR, DP_ANDS_STR, 1},
        {DP_MVN_STR, DP_ORN_STR, 2},
        {DP_MOV_STR, DP_ORR_STR, 2},
        {DP_MUL_STR, DP_MADD_STR, 4},
        {DP_MNEG_STR, DP_MSUB_STR, 4},
};

/**
 * Array of entries for mapping arithmetic mnemonics to their opc
 */
static SymbolTableEntry arithmeticEntries[] = {
    {DP_ADD_STR, ADD},
    {DP_ADDS_STR, ADDS},
    {DP_SUB_STR, SUB},
    {DP_SUBS_STR, SUBS},
};

/**
 * Table containing the array of arithmetic entries, its size and its capacity
 */
static SymbolTable arithmeticTable = {arithmeticEntries, 4, 4};

/**
 * Array of entries mapping wide move mnemonics to their opc
 */
static SymbolTableEntry wideMoveEntries[] = {
    {DP_MOVN_STR, MOVN_OPC},
    {DP_MOVZ_STR, MOVZ_OPC},
    {DP_MOVK_STR, MOVK_OPC},
};

/**
 * Table containing the array of wide move entries, its size and its capacity
 */
static SymbolTable wideMoveTable = {wideMoveEntries, 3, 3};

/**
 * Array of entries mapping logic mnemonics to a combination of their opc and N
 */
static SymbolTableEntry logicEntries[] = {
    {DP_AND_STR, AND},
    {DP_BIC_STR, BIC},
    {DP_ORR_STR, ORR},
    {DP_ORN_STR, ORN},
    {DP_EOR_STR, EOR},
    {DP_EON_STR, EON},
    {DP_ANDS_STR, ANDS},
    {DP_BICS_STR, BICS},
};

/**
 * Table containing the array of logic entries, its size and its capacity
 */
static SymbolTable logicTable = {logicEntries, 8, 8};

/**
 * Enum representing the encoding of the 4 different types of shift
 */
typedef enum {
    LSL,
    LSR,
    ASR,
    ROR,
} ShiftType;

/**
 * Array of entries mapping shift mnemonics to their enum encoding
 */
static SymbolTableEntry shiftEntries[] = {
    {LSL_STR, LSL},
    {LSR_STR, LSR},
    {ASR_STR, ASR},
    {ROR_STR, ROR},
};

/**
 * Table containing the array of shift entries, its size and its capacity
 */
static SymbolTable shiftTable = {shiftEntries, 4, 4};

/**
 * Array of entries mapping branch condition mnemonics to their encoding
 */
static SymbolTableEntry branchEntries[] = {
    {EQ_STR, EQ},
    {NE_STR, NE},
    {GE_STR, GE},
    {LT_STR, LT},
    {GT_STR, GT},
    {LE_STR, LE},
    {AL_STR, AL},
};

/**
 * Table containing the array of branch condition entries, its size and capacity
 */
static SymbolTable branchTable = {branchEntries, 7, 7};

/**
 * Replaces an alias with a given mnemonic and inserts the token representation
 * of the zero register ("wzr"/"xzr" depending on bit mode) at a given index in
 * the token array
 * Returns -1 if memory allocation fails, otherwise returns 0 
 */
static int parse_alias(TokenisedString *, char *, int, int);

/**
 * Parses a tokenised data processing immediate instruction into an internal
 * representation of an instruction
 */
static void parse_dp_immediate(TokenisedString *, Instr *, int);

/**
 * Parses a tokenised data processing register instruction into an internal
 * representation of an instruction
 */
static void parse_dp_register(TokenisedString *, Instr *, int);

/**
 * Parses a tokenised single data transfer instruction into an internal
 * representation of an instruction
 */
static void parse_single_data_transfer(TokenisedString *, SymbolTable *, uint32_t, Instr *);

/**
 * Parses a tokenised branch instruction into an internal representation of an
 * instruction
 */
static void parse_branch(TokenisedString *, SymbolTable *, uint32_t, Instr *);

/**
 * Returns true if a given string is an immediate value (format: "#N" or "#0xN")
 */
static bool is_immediate(char *);

/**
 * Parses a string representing an immediate value into an integer
 */
static uint32_t parse_immediate(char *);

/**
 * Parses a string representing a register into a register index and bit mode
 */
static void parse_register(char *, ParsedRegister *);

////////////////////////////////////////////////////////////////////////////////

/**
 * Pointer to a function that parses a tokenised instruction into an internal
 * representation of an instruction
 */
typedef void (*ParsePtr)(TokenisedString *, SymbolTable *, uint32_t, Instr *);

/**
 * Pointer to a function that parses a tokenised data processing instruction
 * into an internal representation of an instruction
 */
typedef void (*ParseDPPtr)(TokenisedString *, Instr *, int);

/**
 * Struct containing an array of mnemonics, the number of mnemonics, a pointer
 * to a parse function (nullable), a pointer to a parse data processing function
 * (nullable) and an enum value representing the type of data processing
 */
typedef struct {
    int num_mnemonics;
    char mnemonics[10][MAX_TOKEN_SIZE];
    ParsePtr parse_ptr;
    ParseDPPtr parse_dp_ptr;
    int dp_type;
} ParseTableEntry;

/**
 * Table mapping an array of mnemonics to a shared parse function (pointed to by
 * ParsePtr)
 * For data processing mnemonics excluding arithemtic, parse_ptr = NULL,
 * parse_dp_ptr = pointer to data processing parse function (imm/reg), and
 * dp_type = type of data processing instruction
 * For data processing arithmetic mnemonics, parse_ptr = parse_dp_ptr = NULL
 */
static ParseTableEntry parseTable[] = {
    {4,{DP_ADD_STR, DP_ADDS_STR, DP_SUB_STR, DP_SUBS_STR}, NULL, NULL, -1},
    {3, {DP_MOVK_STR, DP_MOVN_STR, DP_MOVZ_STR}, NULL, &parse_dp_immediate, IMM_WIDE_MOVE},
    {8, {DP_AND_STR, DP_ANDS_STR, DP_BIC_STR, DP_BICS_STR, DP_EOR_STR, DP_ORR_STR, DP_EON_STR, DP_ORN_STR}, NULL, &parse_dp_register, REG_LOGICAL},
    {2, {DP_MADD_STR, DP_MSUB_STR}, NULL, &parse_dp_register, REG_MULTIPLY},
    {2, {SDT_LDR_STR, SDT_STR_STR}, &parse_single_data_transfer, NULL, -1},
    {3, {BRANCH_STR, BRANCH_REG_STR}, &parse_branch, NULL, -1},
};

////////////////////////////////////////////////////////////////////////////////

/**
 * Parses a tokenised string into an internal representation of an instruction
 * If the mnemonic is an alias, calls parse_alias
 * Then loops through the parseTable to find a specialised helper function to
 * delegate the parsing to
 */
int parse(TokenisedString *tokenised, SymbolTable *labels, uint32_t current_address, Instr *instr) {
    char *mnemonic = tokenised->tokens[MNEMONIC_INDEX];

    // Checks whether the instruction mnemonic is an alias
    for (int i = 0; i < sizeof(aliasTable) / sizeof(aliasTable[0]); i++) {
        // If mnemonic is an alias, replaces it with its corresponding mnemonic
        if (strcmp(mnemonic, aliasTable[i].alias) == 0) {
            ParsedRegister reg;
            parse_register(tokenised->tokens[1], &reg);
            if (parse_alias(tokenised, aliasTable[i].mnemonic, aliasTable[i].index, reg.bit_mode) != 0) {
                return -1;
            }
        }
    }

    // Maps the mnemonic to a parse function (dp imm/reg, single data transfer, branch)
    for (int i = 0; i < sizeof(parseTable) / sizeof(parseTable[0]); i++) {
        for (int j = 0; j < parseTable[i].num_mnemonics; j++) {
            ParseTableEntry entry = parseTable[i];
            if (strcmp(entry.mnemonics[j], mnemonic) == 0) {
                if (entry.parse_ptr != NULL) {
                    // If parse_ptr is not null (instr is not dp), calls it
                    entry.parse_ptr(tokenised, labels, current_address, instr);
                } else if (entry.parse_dp_ptr != NULL) {
                    // If parse_dp_ptr is not null (instr is dp but not arithmetic), calls it
                    entry.parse_dp_ptr(tokenised, instr, entry.dp_type);
                } else {
                    // Otherwise, instruction is arithmetic
                    // Determines whether it is imm or reg and calls the appropriate function
                    if (is_immediate(tokenised->tokens[3])) {
                        parse_dp_immediate(tokenised, instr, IMM_ARITHMETIC);
                    } else {
                        parse_dp_register(tokenised, instr, REG_ARITHMETIC);
                    }
                }
                return 0;
            }
        }
    }

    return -1;
}

static int parse_alias(TokenisedString *tokenised, char *mnemonic, int index, int bit_mode) {
    // Replaces the alias with its corresponding mnemonic
    strcpy(tokenised->tokens[MNEMONIC_INDEX], mnemonic);
    
    // Dynamically allocates memory for the new token
    if ((tokenised->tokens[tokenised->num_tokens] = malloc(MAX_TOKEN_SIZE * sizeof(char))) == NULL) {
        return -1;
    }

    // Shifts tokens after and including the index to the right by 1
    for (int i = tokenised->num_tokens; i > index; i--) {
        strcpy(tokenised->tokens[i], tokenised->tokens[i - 1]); 
    }

    // Copies zero register ("wzr"/"xzr" according to bit mode) into index
    tokenised->tokens[index][0] = bit_mode == BIT_MODE_32 ? CHAR_32_BIT : CHAR_64_BIT;
    strcpy(tokenised->tokens[index] + 1, ZERO_REG_STR);

    // Increments number of tokens
    tokenised->num_tokens++;

    return 0;
}

static void parse_dp_immediate(TokenisedString *tokenised, Instr *instr, int type) {
    DPImmFormat format;

    char **tokens = tokenised->tokens;
    char *mnemonic = tokens[MNEMONIC_INDEX];

    // Sets the imm_type, rd and sf fields
    format.imm_type = type;
    ParsedRegister reg;
    parse_register(tokens[1], &reg);
    format.rd = reg.index;
    format.sf = reg.bit_mode;

    switch (type) {
        case IMM_ARITHMETIC:
            // Arithmetic: sets the opi, opc, rn, imm12 and sh fields
            format.opi = ARITHMETIC_OPI;
            format.opc = lookup(&arithmeticTable, mnemonic);
            parse_register(tokens[2], &reg);
            format.operand.Arithmetic.rn = reg.index;
            format.operand.Arithmetic.imm12 = parse_immediate(tokens[3]);
            format.operand.Arithmetic.sh = (tokenised->num_tokens == 4 ? 0 : parse_immediate(tokens[5])) / IMM12_LENGTH;
            break;
        case IMM_WIDE_MOVE:
            // Wide Move: sets the opi, opc, imm16 and hw fields
            format.opi = WIDE_MOVE_OPI;
            format.opc = lookup(&wideMoveTable, mnemonic);
            format.operand.WideMove.imm16 = parse_immediate(tokens[2]);
            format.operand.WideMove.hw = (tokenised->num_tokens == 3 ? 0 : parse_immediate(tokens[4])) / 16;
            break;
        default:
            // Assume valid instruction type - should not reach this case
            assert(0);
    }

    // Sets the type and format of the parsed data processing (imm.) instruction
    instr->type = DATA_PROCESSING_IMM;
    instr->format.dp_imm_format = format;
}

static void parse_dp_register(TokenisedString *tokenised, Instr *instr, int type) {
    DPRegFormat format;

    char **tokens = tokenised->tokens;
    char *mnemonic = tokens[MNEMONIC_INDEX];

    // Sets the reg_type, M, rd, rn, rm and sf fields
    format.reg_type = type;
    format.M = type == REG_MULTIPLY;
    ParsedRegister reg;
    int num_regs = 3;
    char regs[num_regs];
    for (int i = 0; i < num_regs; i++) {
        parse_register(tokens[i + 1], &reg);
        regs[i] = reg.index;
    }
    format.rd = regs[0];
    format.rn = regs[1];
    format.rm = regs[2];
    format.sf = reg.bit_mode;

    // Multiply format: m<add/sub> rd rn rm ra
    if (type == REG_MULTIPLY) {
        format.operand.multiply_operand.x = strcmp(mnemonic, DP_MADD_STR) == 0 ?
            MULTIPLY_ADD_X : MULTIPLY_SUB_X;
        parse_register(tokens[4], &reg);
        format.operand.multiply_operand.ra = reg.index;
    } else {
        int arithmetic_logical_shift = tokenised->num_tokens == num_regs + 1 ? 0 : lookup(&shiftTable, tokens[4]);
        int arithmetic_logical_operand =  tokenised->num_tokens == num_regs + 1 ? 0 : parse_immediate(tokens[5]);
        
        switch (type) {
            case REG_ARITHMETIC:
                // Arithmetic format: mnemonic rd rn rm {shift #amount}
                format.opc = lookup(&arithmeticTable, mnemonic);
                format.opr.arithmetic_shift = arithmetic_logical_shift;
                format.operand.arithmetic_logical_operand = arithmetic_logical_operand;
                break;
            case REG_LOGICAL: ;
                // Logical format: mnemonic rd rn rm {shift #amount}
                uint32_t opc_N = lookup(&logicTable, mnemonic);
                format.opc = opc_N >> OPC_SHIFT;
                format.opr.logical_opr.N = opc_N & N_MASK;
                format.opr.logical_opr.shift = arithmetic_logical_shift;
                format.operand.arithmetic_logical_operand = arithmetic_logical_operand;
                break;
            default:
                // Assume valid instruction type - should not reach this case
                assert(0);
        }
    }

    // Sets the type and format of the parsed data processing (reg.) instruction
    instr->type = DATA_PROCESSING_REG;
    instr->format.dp_reg_format = format;
}

static void parse_single_data_transfer(TokenisedString *tokenised, SymbolTable *labels, uint32_t current_address, Instr *instr) {
    SDTFormat format;
    ParsedRegister reg;

    char **tokens = tokenised->tokens;

    // Parses rt (target register) and sf (bit mode) fields
    parse_register(tokens[1], &reg);
    format.rt = reg.index;  
    format.sf = reg.bit_mode;

    // Determines the subtype of the instruction - SDT or Load Literal
    if (tokens[2][0] == '[') {
        format.sdt_type = SDT;
        format.SDT.L = strcmp(tokens[MNEMONIC_INDEX], SDT_LDR_STR) == 0;
        parse_register(strtok(tokens[2], "[]"), &reg);
        format.SDT.xn = reg.index;

        // Sets the single data transfer addressing mode and offset fields
        if (tokenised->num_tokens == 3) {
            // Handles the case where the instruction is zero unsigned offset
            format.SDT.addr_mode = UNSIGNED_OFFSET;
            format.SDT.offset.imm12 = 0;
        } else {
            if (is_immediate(tokens[3])) {
                uint32_t offset = parse_immediate(tokens[3]);
                switch (tokens[3][strlen(tokens[3]) - 1]) {
                    case ']':
                        format.SDT.addr_mode = UNSIGNED_OFFSET;
                        // Divides the offset by either 4 or 8 depending on the bit mode
                        format.SDT.offset.imm12 = offset * (CHAR_BIT) /
                            (reg.bit_mode == BIT_MODE_32 ? BIT_SIZE_32 : BIT_SIZE_64); 
                        break;
                    case '!':
                        format.SDT.addr_mode = PRE_INDEX;
                        format.SDT.offset.simm9 = offset;
                        break;
                    default:
                        format.SDT.addr_mode = POST_INDEX;
                        format.SDT.offset.simm9 = offset;
                }
            } else {
                format.SDT.addr_mode = REGISTER_OFFSET;
                parse_register(strtok(tokens[3], "]!"), &reg);
                format.SDT.offset.xm = reg.index; 
            }
        }
    } else {
        format.sdt_type = LOAD_LITERAL;
        format.LoadLiteral.simm19 = is_immediate(tokens[2]) ? parse_immediate(tokens[2])
            : (lookup(labels, tokens[2]) - current_address) / INSTR_BYTES;
    }

    // Sets the type and format of the parsed single data transfer instruction
    instr->type = SINGLE_DATA_TRANSFER;
    instr->format.sdt_format = format;
}

static void parse_branch(TokenisedString *tokenised, SymbolTable *labels, uint32_t current_address, Instr *instr) {
    BranchFormat format;
    ParsedRegister reg;

    char **tokens = tokenised->tokens;
    char *mnemonic = tokens[MNEMONIC_INDEX];

    // Sets the branch type and address offset fields
    if (strcmp(mnemonic, BRANCH_STR) == 0) {
        if (tokenised->num_tokens == 2) {
            format.branch_type = UNCONDITIONAL;
            // Calculates the offset from the address of the current instruction
            format.Unconditional.simm26 = (lookup(labels, tokens[1]) - current_address) / INSTR_BYTES;
        } else {
            format.branch_type = CONDITIONAL;
            format.Conditional.cond = lookup(&branchTable, tokens[1]);
            // Calculates the offset from the address of the current instruction
            format.Conditional.simm19 = (lookup(labels, tokens[2]) - current_address) / INSTR_BYTES;
        }
    } else if (strcmp(mnemonic, BRANCH_REG_STR) == 0) {
        format.branch_type = REGISTER;
        parse_register(tokens[1], &reg);
        format.Register.xn = reg.index;
    } else {
        // Assume valid instruction type - should not reach this case
        assert(0);  
    }

    // Sets the type and format of the parsed branch instruction
    instr->type = BRANCH;
    instr->format.branch_format = format;
}

static bool is_immediate(char *token) {
    return token[0] == '#';
}

static uint32_t parse_immediate(char *token) {
    uint32_t imm;
    // Handles both hex and decimal immediate values
    if (sscanf(token, "#0x%x", &imm) != 1) {
        sscanf(token, "#%u", &imm);
    }
    return imm;
}

static void parse_register(char *token, ParsedRegister *reg) {
    reg->bit_mode = token[0] == CHAR_32_BIT ? BIT_MODE_32 : BIT_MODE_64;
    
    if (strcmp(token + 1, ZERO_REG_STR) == 0) {
        // If "zr" is in the token, sets register index to zero register (31)
        reg->index = ZERO_REG_INDEX;
    } else {
        // Otherwise casts the rest of the token (after the 'x'/'w') to an int
        reg->index = atoi(token + 1);
    }
}