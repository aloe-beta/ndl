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
GroupNode *parse_group(Parser *p) {
    return NULL;
}

SimpleNode *parse_simple(Parser *p) {
    Token *start_pos = p->pos;
    char *arena_pos = p->arena.pos;

    SimpleNode node;
    node.type = T_SIMPLE;

    node.lookbehind = unknown_matches(p, '<');
    if (node.lookbehind) p->pos++;

    node.negate = unknown_matches(p, '~');
    if (node.negate) p->pos++;

    TokenType type = p->pos->type;
    if (type == T_STRING || type == T_RANGE || type == T_LEX_IDEN || type == T_PARSE_IDEN) {
        TokenNode token_node;
        token_node.type = T_TOKEN;
        token_node.token = p->pos++;

        TokenNode *ptr = (TokenNode*) arena_alloc(&p->arena, sizeof(TokenNode));
        if (ptr == NULL) goto fail;
        *ptr = token_node;

        node.match = (NodeType*) (p->arena.ptr - (char*) ptr);
    } else {
        node.match = (NodeType*) parse_group(p);
        if (node.match == NULL) goto fail;
    }

    node.iden = NULL;
    node.flag = unknown_matches(p, '@');
    if (node.flag || unknown_matches(p, ':')) {
        p->pos++;

        if (p->pos->type != T_PARSE_IDEN) goto fail;
        node.iden = p->pos++;
    }

    node.quantity = '\0';
    node.delimiter = NULL;

    if (unknown_matches(p, '?')) {
        p->pos++;
        node.quantity = '?';
    } else if (unknown_matches(p, '*') || unknown_matches(p, '+')) {
        node.quantity = *p->pos->start;
        p->pos++;

        if (unknown_matches(p, ':')) {
            p->pos++;

            node.delimiter = (NodeType*) parse_simple(p);
            if (node.delimiter == NULL) goto fail;
        }
    }

    node.lookahead = unknown_matches(p, '>');
    if (node.lookahead) p->pos++;

    SimpleNode *ptr = (SimpleNode*) arena_alloc(&p->arena, sizeof(SimpleNode));
    if (ptr == NULL) goto fail;
    *ptr = node;

    return (SimpleNode*) (p->arena.ptr - (char*) ptr);

fail:
    p->pos = start_pos;
    // TODO: Clean up instead of stepping back alone.
    p->arena.pos = arena_pos;
    return NULL;
}

// TODO: Implement
MatchNode *parse_match(Parser *p) {
    return NULL;
}

TokenDefNode *parse_tokendef(Parser *p) {
    Token *start_pos = p->pos;
    TokenDefNode node;
    node.type = T_TOKENDEF;

    node.skip = unknown_matches(p, '~');
    if (node.skip) p->pos++;

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