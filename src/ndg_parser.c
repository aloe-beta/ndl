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
    p->pos = tokens;
}

static char unknown_matches(Parser *p, char c) {
    return p->pos->type == T_UNKNOWN && *p->pos->start == c;
}

// Note: Parsing functions should return relative pointers to allow arena realloc.
// Note: OOM may result in odd parsing instead of exiting with an error.

// TODO: Implement
MatchNode *parse_match(Parser *p) {
    return NULL;
}

TokenDefNode *parse_tokendef(Parser *p) {
    Token *start_pos = p->pos;
    TokenDefNode node;
    node.skip = false;

    if (unknown_matches(p, '~')) {
        p->pos++;
        node.skip = true;
    }

    if (p->pos->type != T_LEX_IDEN) goto fail;

    node.name = p->pos++;

    node.overrides = NULL;
    if (unknown_matches(p, ':')) {
        p->pos++;
        
        if (p->pos->type != T_LEX_IDEN) goto fail;

        node.overrides = p->pos++;
    }

    if (!unknown_matches(p, '=')) goto fail;
    node.match = parse_match(p);
    if (node.match == NULL || !unknown_matches(p, ';')) goto fail;

    TokenDefNode *ptr = (TokenDefNode*) arena_alloc(&p->arena, sizeof(TokenDefNode));
    if (ptr == NULL) goto fail;
    *ptr = node;

    return (TokenDefNode*) (p->arena.ptr - (char*) ptr);

fail:
    p->pos = start_pos;
    return NULL;
}

// TODO: Implement
void parse_grammar(Parser *p) {
    while (unknown_matches(p, ';')) p->pos++;
}