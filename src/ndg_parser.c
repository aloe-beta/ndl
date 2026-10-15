#include "ndg.h"

void init_arena(Arena *a, size_t size) {
    char *ptr = malloc(size);
    a->ptr = ptr;
    a->pos = ptr;
    a->size = size;
}

void init_parser(Parser *p, Token *tokens) {
    Arena a;
    init_arena(&a, 16384);

    p->arena = a;
    p->first = NULL;
    p->tokens = tokens;
}

void parse_grammar() {}