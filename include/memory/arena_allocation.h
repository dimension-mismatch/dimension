#pragma once
#include <stdint.h>

typedef struct{
    void* data;
    void* cursor;
    void* end;
} arena_t;

arena_t arena_init(size_t byte_count);

void* arena_malloc(arena_t* arena, size_t byte_count);

void arena_destroy(arena_t* arena);
