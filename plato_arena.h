#ifndef PLATO_ARENA_H
#define PLATO_ARENA_H

#include <stdlib.h>
#include <string.h>

#define PL_ARENA_MEM_ALIGN(p, a) ((p + (a - 1)) & ~(a - 1))

typedef struct pl_arena_s {
    void *data;
    size_t capacity;
    size_t offset;
} pl_arena_t;

int pl_arena_init(pl_arena_t *arena, size_t capacity);
void *pl_arena_alloc(pl_arena_t *arena, size_t size);
void *pl_arena_aligned_alloc(pl_arena_t *arena, size_t size, size_t alignment);
void pl_arena_free(pl_arena_t *arena);
void pl_arena_reset(pl_arena_t *arena);

#if defined(PLATO_IMPLEMENTATION) || defined(PLATO_ARENA_IMPLEMENTATION)

pl_arena_t *pl_arena_init(size_t capacity) {
    pl_arena_t *arena = malloc(sizeof(pl_arena_t));
    if(!arena) return NULL;

    arena->data = malloc(capacity);
    if(!arena->data) return NULL;

    arena->capacity = capacity;
    arena->offset = 0;
    return arena;
}

void *pl_arena_alloc(pl_arena_t *arena, size_t size) {
    if(!arena || !arena->data) return NULL;

    size_t aligned_offset = PL_ARENA_MEM_ALIGN(arena->offset, sizeof(void*));
    if(aligned_offset + size > arena->capacity) return NULL;

    void *ptr = (void*)((char*)arena->data + aligned_offset);
    arena->offset = aligned_offset + size;
    memset(ptr, 0, size);

    return ptr;
}

void *pl_arena_aligned_alloc(pl_arena_t *arena, size_t size, size_t alignment) {
    if(!arena || !arena->data || alignment <= 0) return NULL;
    if((alignment & (alignment - 1)) != 0) return NULL;

    size_t aligned_offset = PL_ARENA_MEM_ALIGN(arena->offset, alignment);
    if(aligned_offset + size > arena->capacity) return NULL;

    void *ptr = (void*)((char*)arena->data + aligned_offset);
    arena->offset = aligned_offset + size;
    memset(ptr, 0, size);

    return ptr;
}

void pl_arena_free(pl_arena_t *arena) {
    if(arena && arena->data) {
        free(arena->data);
        free(arena);
    }
}

void pl_arena_reset(pl_arena_t *arena) {
    if(arena) arena->offset = 0;
}

#endif // PLATO_ARENA_IMPLEMENTATION
#endif // PLATO_ARENA_H