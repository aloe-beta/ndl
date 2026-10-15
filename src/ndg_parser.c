#include "ndg.h"

void init_arena(Arena *a, size_t size) {
    char *ptr = malloc(size);
    a->ptr = ptr;
    a->pos = ptr;
    a->size = size;
}

char *arena_alloc(Arena *a, size_t size) {
    size = (size + 7) & ~7;

    size_t offset = a->pos - a->ptr;
    if (offset + size > a->size) {
        size_t new_size = (a->size == 0) ? 1024 : a->size;

        while (offset + size > new_size) new_size *= 2;
        char *ptr = realloc(a->ptr, new_size);

        if (ptr == NULL) {
            return NULL;
        }

        a->ptr = ptr;
        a->pos = a->ptr + offset;
        a->size = new_size;
    }

    char* pos = a->pos;
    a->pos += size;

    return pos;
}

void init_parser(Parser *p, Token *tokens) {
    init_arena(&p->arena, 16384);

    p->first = NULL;
    p->tokens = tokens;
}

void parse_grammar() {}