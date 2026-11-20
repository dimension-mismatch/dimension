#include <memory/arena_allocation.h>

#include <stdlib.h>
#include <stdio.h>


arena_t arena_init(size_t byte_count){
    void* data = malloc(byte_count);
    if(!data){
        printf("Failed to allocate %zu bytes for arena", byte_count);
    }
    arena_t new = {.data = data, .cursor = data, .end = (uint8_t*)data + byte_count}; 
    return new;
}

void* arena_malloc(arena_t* arena,size_t byte_count){
    void* temp = arena->cursor;
    arena->cursor += byte_count;
    return temp;
}

void arena_destroy(arena_t* arena){
    free(arena->data);
    arena->data = NULL;
    arena->cursor = NULL;
    arena->end = NULL;
}