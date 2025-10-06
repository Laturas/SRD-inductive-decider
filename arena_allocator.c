#include <stdlib.h>
#ifndef DEFINITIONS
    #include "defs.h"
#endif

typedef struct Arena {
    usize underlying_allocation_amount;
    usize number_of_bytes_in_use;

    u8* bytes;
} Arena;

// You can swap out these functions with any allocator of your choice.
[[no_discard]] void* SYSTEM_DEPENDENT_underlying_allocator(usize byte_count) {
    return malloc(byte_count);
}
[[no_discard]] void* SYSTEM_DEPENDENT_underlying_re_allocator(void* original, usize byte_count) {
    return realloc(original, byte_count);
}
[[no_discard]] void* SYSTEM_DEPENDENT_underlying_free(void* to_free) {
    free(to_free);
    return NULL;
}

Arena arena_init(usize byte_count) {
    u8* bytes = SYSTEM_DEPENDENT_underlying_allocator(byte_count);
    Arena final_arena = {
        byte_count,
        0,
        bytes
    };
    return final_arena;
}

/// For internal usage only
Arena _arena_expand(Arena old_arena, usize byte_count) {
    u8* bytes = SYSTEM_DEPENDENT_underlying_re_allocator(old_arena.bytes, byte_count);
    Arena final_arena = {
        byte_count,
        old_arena.number_of_bytes_in_use,
        bytes
    };
    return final_arena;
}

/// Empties the arena without freeing the underlying data.
void aclear(Arena* old_arena) {
    old_arena->underlying_allocation_amount = 0;
    old_arena->number_of_bytes_in_use = 0;
}

// Frees the underlying memory of the arena and zeros it out
void afree(Arena* arena_to_free) {
    arena_to_free->bytes = SYSTEM_DEPENDENT_underlying_free(arena_to_free->bytes);
    arena_to_free->underlying_allocation_amount = 0;
    arena_to_free->number_of_bytes_in_use = 0;
}

/// Allocates some number of bytes onto a given arena
void* aalloc(Arena* arena, usize byte_count) {
    assert("Empty arena passed into aalloc", arena->underlying_allocation_amount != 0);
    while (arena->number_of_bytes_in_use + byte_count >= arena->underlying_allocation_amount) {
        arena->bytes = SYSTEM_DEPENDENT_underlying_re_allocator(arena->bytes, arena->underlying_allocation_amount * 2);
        arena->underlying_allocation_amount *= 2;
    }
    u8* allocated_bytes = &arena->bytes[byte_count];
    arena->number_of_bytes_in_use += byte_count;
    return allocated_bytes;
}