#ifndef DATA_STRUCTURES
#define DATA_STRUCTURES
#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#include "c_datastructures.h"

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

#define TABLE_SPACE 16

ht_table* create_table() {
	ht_table* table = malloc(sizeof(ht_table));
	if (table == NULL) {return NULL;}

	table->space = TABLE_SPACE;
	table->size = 0;

	table->entries = calloc(table->space, sizeof(ht_entry));

	if (table->entries == NULL) {
		free(table);
		return NULL;
	}

	return table;
}

void table_destroy(ht_table* table) {
	for(size_t i = 0; i < table->space; i++) {
		free((void*)table->entries[i].key);
	}

	free(table->entries);
	free(table);
}

size_t table_get_keys(ht_table* table, char* key_arr[]) {
	size_t count = 0;
	for (size_t i = 0; i < table->size; i++) {
		if (table->entries[i].key) {
			printf("Key that will be passed in get keys: %s\n", table->entries[i].key);
			key_arr[count++] = table->entries[i].key;
		}
	}	
	printf("Will pass this ehheeh");
	return count;
}

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

static uint64_t hash_key(const char* key) {
	uint64_t hash = FNV_OFFSET;
	for (const char* p = key; *p; p++) {
		hash ^= (uint64_t)(unsigned char)(*p);
		hash *= FNV_PRIME;
	}

	return hash;
}	

void* table_get(ht_table* table, const char* key) {
		uint64_t hash = hash_key(key);
		size_t index = (size_t)(hash & (uint64_t)(table->space - 1));

		while(table->entries[index].key != NULL) {
				if (strcmp(key, table->entries[index].key) == 0) {
					return table->entries[index].value;
			}
		

		index++;

		if (index >= table->space) {
			index = 0;
		}
	}

	return NULL;
}

static const char* table_set_entry(ht_entry* entries, size_t space, const char* key, void* value, size_t* plength) {
	uint64_t hash =  hash_key(key);
	    size_t index = (size_t)(hash & (uint64_t)(space - 1));

    // Loop till we find an empty entry.
	    while (entries[index].key != NULL) {
        	if (strcmp(key, entries[index].key) == 0) {
            	// Found key (it already exists), update value.
            	entries[index].value = value;
            	return entries[index].key;
        	}
        	// Key wasn't in this slot, move to next (linear probing).
        	index++;
	if (index >= space) {
        	    // At end of entries array, wrap around.
            	index = 0;
        	}
    	}

    // Didn't find key, allocate+copy if needed, then insert it.
	    if (plength != NULL) {
        	key = strdup(key);
        	if (key == NULL) {
            	return NULL;
        	}
        	(*plength)++;
    	}
    	entries[index].key = (char*)key;
    	entries[index].value = value;
    	return key;
	
}

const char* table_set(ht_table* table, const char* key, void* value) {
	assert(value != NULL);
	if (value == NULL) {
		return NULL;
	}

	if (table->size >= table->space) {
		printf("Table full");
		return NULL;
	}

	return table_set_entry(table->entries, table->space, key, value, &table->size);
}
#endif
