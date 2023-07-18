#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "assembler.h"
#include "tokeniser.h"
#include "parser.h"
#include "encoder.h"
#include "symbol_table.h"
#include "mnemonics.h"
#include "../common/utilities.h"
#include "../common/instructions.h"

#define MAX_LINE_LENGTH 100
#define INT_DIRECTIVE_STR ".int"

/**
 * Returns true if a given string is all whitespace
 */
static bool is_empty(char *);

/**
 * Returns a pointer to the first non-whitespace character of a string 
 * Pre: the string is not empty
 */
static char *remove_leading_whitespace(char *);

/**
 * Returns true if a given string is a label definition (format: "label_name:")
 */
static bool is_label_def(char *);

/**
 * Returns true if a given string is a .int directive (format: ".int N")
 */
static bool is_int_directive(char *);

void first_pass(FILE *in, SymbolTable *labels) {
    // Tracks the current line number (ignoring label definitions)
    uint32_t line_num = 0;

    // Buffer for storing a line of the input file
    char buffer[MAX_LINE_LENGTH];

    // Reads the input file line by line
    while (fgets(buffer, MAX_LINE_LENGTH, in) != NULL) {
        // Removes the trailing newline character from the line
        buffer[strcspn(buffer, "\n")] = '\0';

        // Ignores empty lines
        if (!is_empty(buffer)) {
            char *line = buffer;
            line = remove_leading_whitespace(line);

            uint32_t instr_addr = line_num * INSTR_BYTES;

            if (is_label_def(line)) {
                // If line is a label definition, removes the trailing colon
                line[strcspn(line, ":")] = '\0';

                // Adds the label and its corresponding address to symbol table
                insert(labels, line, instr_addr);
            } else {
                // If line is not a label definition, increments the line count
                line_num++;
            }
        }
    }

    // Resets the pointer to the beginning of the file
    fseek(in, 0, SEEK_SET);
}

int second_pass(FILE *in, FILE *out, SymbolTable *labels) {
    // Tracks the current line number (ignoring label definitions)
    uint32_t line_num = 0;

    // Buffer for storing a line of the input file
    char buffer[MAX_LINE_LENGTH];

    // Reads the input file line by line
    while (fgets(buffer, MAX_LINE_LENGTH, in) != NULL) {
        // Removes the trailing newline character from the line
        buffer[strcspn(buffer, "\n")] = '\0';

        // Ignores empty lines
        if (!is_empty(buffer)) {
            char *line = buffer;
            line = remove_leading_whitespace(line);

            // Ignores label definitions
            if (!is_label_def(line)) {
                // Tokenises the line - returns -1 if tokenisation fails
                TokenisedString tokenised;
                if (tokenise(line, &tokenised) != 0) {
                    return -1;
                }

                Instr instr;
                uint32_t encoded;

                if (strcmp(tokenised.tokens[0], NOP_STR) == 0) {
                    // If line is "nop", encodes it directly
                    encoded = NOP_PATTERN;
                } else if (is_int_directive(line)) {
                    // If line is .int directive, encodes its value directly
                    if (sscanf(tokenised.tokens[1], "0x%x", &encoded) != 1) {
                        sscanf(tokenised.tokens[1], "%u", &encoded);
                    }
                } else {
                    // Parses line into internal representation of instruction
                    if (parse(&tokenised, labels, line_num * INSTR_BYTES, &instr) != 0) {
                        // Returns -1 if parsing fails
                        return -1;
                    }

                    // Encodes internal representation of instruction in binary
                    encoded = encode(&instr);
                }

                // Writes the encoded instruction to the binary output file
                fwrite(&encoded, sizeof(uint32_t), 1, out);

                // Frees dynamically allocated memory used for token array
                free_tokens(&tokenised);

                // Increments the line count
                line_num++;
            }
        }
    }
    return 0;
}

static bool is_empty(char *line) {
    while (*line != '\0') {
        if (!isspace(*line)) {
            return false;
        }
        line++;
    }
    return true;
}

static char *remove_leading_whitespace(char *line) {
    while (isspace(*line)) {
        line++;
    }
    return line;
}

static bool is_label_def(char *line) {
    // Checks whether a ':' is present in the line
    return strchr(line, ':') != NULL;
    assert(0);
}

static bool is_int_directive(char *line) {
    // Checks whether ".int" is present in the line
    return strstr(line, INT_DIRECTIVE_STR) != NULL;
}