#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdint.h>

/**
 * Represents an entry in a symbol table, where key = string (label) and
 * value = int (address)
 */
typedef struct {
    char *str;
    uint32_t value;
} SymbolTableEntry;

/**
 * Represents a symbol table, containing an array of entries, the current size
 * and the table capacity
 */
typedef struct {
    SymbolTableEntry *entries;
    int size;
    int capacity;
} SymbolTable;

/**
 * Initialises a symbol table with size = 0 and a given capacity, dynamically
 * allocating memory for the array of table entries
 * Returns 0 for success, -1 for failure
 */
extern int initialise_table(SymbolTable *, int);

/**
 * Adds a new key-value pair to the symbol table
 * Allocates more memory if the table is already at full capacity
 * Returns 0 for success, -1 for failure
 */
extern int insert(SymbolTable *, char *, uint32_t);

/**
 * Looks up a string in a given symbol table & returns its associated int value
 * Pre: the string exists in the table
 */
extern uint32_t lookup(SymbolTable *, char *);

/**
 * Frees the memory dynamically allocated by the table
 */
extern void free_table(SymbolTable *);

#endif