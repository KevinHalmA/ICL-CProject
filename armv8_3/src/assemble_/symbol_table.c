#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "symbol_table.h"

int initialise_table(SymbolTable *sym_table, int capacity) {
    // Returns -1 if initial capacity is 0
    if (capacity == 0) {
        return -1;
    }
    
    // Returns -1 if memory cannot be allocated for the table entries
    if ((sym_table->entries = malloc(capacity * sizeof(SymbolTableEntry))) == NULL) {
        return -1;
    }

    sym_table->size = 0;
    sym_table->capacity = capacity;
    
    return 0;
}

int insert(SymbolTable *sym_table, char *str, uint32_t value) {
    // If the table is at full capacity, attempts to allocate more memory 
    if (sym_table->size == sym_table->capacity) {
        int new_capacity = sym_table->capacity * 2;
        if ((sym_table->entries = realloc(sym_table->entries, new_capacity * sizeof(SymbolTableEntry))) == NULL) {
            // Returns -1 (failure) if more memory cannot be allocated
            return -1;
        }
        sym_table->capacity = new_capacity;
    }

    char *str_copy;
    // Dynamically allocates memory for the string and duplicates it
    if ((str_copy = strdup(str)) == NULL) {
        // Returns -1 (failure) if more memory cannot be allocated
        return -1;
    }

    // Sets the key and value of new entry and adds it to the table
    SymbolTableEntry entry = {str_copy, value};
    sym_table->entries[sym_table->size] = entry;
    sym_table->size++;

    return 0;
}

uint32_t lookup(SymbolTable *sym_table, char *label) {
    // Iterates through the table entries
    for (int i = 0; i < sym_table->size; i++) {
        if (strcmp(sym_table->entries[i].str, label) == 0) {
            return sym_table->entries[i].value;
        }
    }
    // Assume the label exists in the table - should not reach this line
    assert(0);
}

void free_table(SymbolTable *sym_table) {
    for (int i = 0; i < sym_table->size; i++) {
        // Frees all dynamically allocated strings
        free(sym_table->entries[i].str);
    }
    // Frees the table entries
    free(sym_table->entries);
}