#ifndef PLATO_HASHMAP_H
#define PLATO_HASHMAP_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct pl_hashmap_entry_s {
    const char *key;
    void *value;
} pl_hashmap_entry_t;

typedef struct pl_hashmap_s {
    pl_hashmap_entry_t* entries;
    size_t capacity;
    size_t length;
} pl_hashmap_t;

typedef struct pl_hashmap_iter_s{
    const char* key;
    void* value;
    pl_hashmap_t* _hm;
    size_t _index;
} pl_hashmap_iter_t;

pl_hashmap_t *pl_hashmap_init(void);
void pl_hashmap_destroy(pl_hashmap_t* hm);
void* pl_hashmap_get(pl_hashmap_t* hm, const char* key);
const char* pl_hashmap_set(pl_hashmap_t* hm, const char* key, void* value);
size_t pl_hashmap_len(pl_hashmap_t* hm);
pl_hashmap_iter_t pl_hashmap_iter(pl_hashmap_t* hm);
int pl_hashmap_next(pl_hashmap_iter_t* iter);

#if defined(PLATO_IMPLEMENTATION) || defined(PLATO_HASHMAP_IMPLEMENTATION)

#define _PL_HASHMAP_INITIAL_CAPACITY 16
#define _FNV_OFFSET 14695981039346656037UL
#define _FNV_PRIME 1099511628211UL

pl_hashmap_t *pl_hashmap_init(void) {
    pl_hashmap_t *hm = (pl_hashmap_t*)malloc(sizeof(pl_hashmap_t));
    if(!hm) return NULL;
    hm->length = 0;
    hm->capacity = _PL_HASHMAP_INITIAL_CAPACITY;

    hm->entries = (pl_hashmap_entry_t*)calloc(hm->capacity, sizeof(pl_hashmap_entry_t));
    if(!hm->entries) {
        free(hm);
        return NULL;
    }
    return hm;
}

void pl_hashmap_destroy(pl_hashmap_t *hm) {
    for (size_t i = 0; i < hm->capacity; i++) free((void*)hm->entries[i].key);
    free(hm->entries);
    free(hm);
}

static uint64_t _pl_hash_key_internal(const char *key) {
    uint64_t hash = _FNV_OFFSET;
    for(const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= _FNV_PRIME;
    }
    return hash;
}

void* pl_hashmap_get(pl_hashmap_t *hm, const char *key) {
    uint64_t hash = _pl_hash_key_internal(key);
    size_t index = (size_t)(hash & (uint64_t)(hm->capacity - 1));

    while(hm->entries[index].key != NULL) {
        if(strcmp(key, hm->entries[index].key) == 0) return hm->entries[index].value;
        index++;
        if(index >= hm->capacity) index = 0;
    }
    return NULL;
}

static const char* _pl_hashmap_set_entry_internal(
    pl_hashmap_entry_t *entries, 
    size_t capacity,
    const char *key, 
    void* value, 
    size_t *plength
) {
    uint64_t hash = _pl_hash_key_internal(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    while(entries[index].key != NULL) {
        if(strcmp(key, entries[index].key) == 0) {
            entries[index].value = value;
            return entries[index].key;
        }
        index++;
        if(index >= capacity) index = 0;
    }

    if(plength != NULL) {
        key = strdup(key);
        if(key == NULL) return NULL;
        (*plength)++;
    }
    entries[index].key = (char*)key;
    entries[index].value = value;
    return key;
}

static int _pl_hashmap_expand_internal(pl_hashmap_t *hm) {
    size_t new_capacity = hm->capacity * 2;
    if(new_capacity < hm->capacity) return 1;
    pl_hashmap_entry_t *new_entries = (pl_hashmap_entry_t*)calloc(new_capacity, sizeof(pl_hashmap_entry_t));
    if(new_entries == NULL) return 1;

    for(size_t i = 0; i < hm->capacity; i++) {
        pl_hashmap_entry_t entry = hm->entries[i];
        if(entry.key != NULL) {
            _pl_hashmap_set_entry_internal(new_entries, new_capacity, entry.key, entry.value, NULL);
        }
    }

    free(hm->entries);
    hm->entries = new_entries;
    hm->capacity = new_capacity;
    return 0;
}

const char* pl_hashmap_set(pl_hashmap_t *hm, const char *key, void *value) {
    if(value == NULL) return NULL;

    if(hm->length >= hm->capacity / 2) {
        if(_pl_hashmap_expand_internal(hm) != 0) return NULL;
    }

    return _pl_hashmap_set_entry_internal(hm->entries, hm->capacity, key, value, &hm->length);
}

size_t pl_hashmap_len(pl_hashmap_t *hm) {
    return hm->length;
}

pl_hashmap_iter_t pl_hashmap_iter(pl_hashmap_t *hm) {
    pl_hashmap_iter_t iter;
    iter._hm = hm;
    iter._index = 0;
    return iter;
}

int pl_hashmap_next(pl_hashmap_iter_t *iter) {
    pl_hashmap_t* hm = iter->_hm;
    while(iter->_index < hm->capacity) {
        size_t i = iter->_index;
        iter->_index++;
        if(hm->entries[i].key != NULL) {
            pl_hashmap_entry_t entry = hm->entries[i];
            iter->key = entry.key;
            iter->value = entry.value;
            return 0;
        }
    }
    return 1;
}

#endif // PLATO_HASHMAP_IMPLEMENTATION
#endif // PLATO_HASHMAP_H