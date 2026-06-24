#ifndef DATA_STRUCTURES
#define DATA_STRUCTURES
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    char *key;
    void *value;
} ht_entry;

typedef struct {
    ht_entry *entries;
    size_t size;
    size_t space;
} ht_table;

typedef ht_table HashTable;
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL
#define TABLE_SPACE 16

ht_table* create_table();
void table_destroy(ht_table* table);
size_t table_get_keys(ht_table* table, char* key_arr[]);
static uint64_t hash_key(const char* key);
void* table_get(ht_table* table, const char* key);
static const char* table_set_entry(ht_entry* entries, size_t space, const char* key, void* value, size_t* plength);
const char* table_set(ht_table* table, const char* key, void* value); 
#endif
