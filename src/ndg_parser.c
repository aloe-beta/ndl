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
    // Add padding to prevent valid relative pointers from being NULL.
    p->arena.pos += sizeof(size_t);

    p->first = NULL;

    p->tokens = tokens;
    p->pos = tokens;
}

static char unknown_matches(Parser *p, char c) {
    return p->pos->type == T_UNKNOWN && *p->pos->start == c;
}

// Note: Parsing functions should return relative pointers to allow arena realloc.
// Note: OOM may result in odd parsing instead of exiting with an error.

size_t *parse_expr(Parser *p) {
    Token *start_pos = p->pos;
    ptrdiff_t arena_offset = p->arena.pos - p->arena.ptr;

    // Allocate arrays of simples
    size_t branch_count = 0;
    while (1) {
        size_t *count = (size_t*) arena_alloc(&p->arena, sizeof(size_t));
        if (count == NULL) goto fail;
        *count = 0;

        while (1) {
            SimpleNode *simple = parse_simple(p);
            if (simple == NULL) break;

            (*count)++;
        }

        if (*count == 0) goto fail;
        branch_count++;

        if (!unknown_matches(p, '|')) break;
        p->pos++;
    }

    // Build pointer list of arrays
    size_t *array = (size_t*) arena_alloc(&p->arena, (branch_count + 1)*sizeof(size_t*));
    if (array == NULL) goto fail;
    size_t *node = (size_t*) ((char*) array - p->arena.ptr);
    *(array++) = branch_count;

    char *scan_ptr = p->arena.ptr + arena_offset;
    for (size_t i = branch_count; i > 0; i--) {
        size_t count = *(size_t*) scan_ptr;
        *(array++) = count;
        scan_ptr += sizeof(size_t) + count*sizeof(SimpleNode);
    }

    return node;

fail:
    p->pos = start_pos;
    // TODO: Clean up
    p->arena.pos = p->arena.ptr + arena_offset;
    return NULL;
}

GroupNode *parse_group(Parser *p) {
    Token *start_pos = p->pos;

    GroupNode node;
    node.type = T_GROUP;

    if (!unknown_matches(p, '(')) goto fail;
    p->pos++;

    node.branch = parse_expr(p);
    if (node.branch == NULL) goto fail;

    if (!unknown_matches(p, ')')) goto fail;
    p->pos++;

    node.delimiter = NULL;
    if (unknown_matches(p, '%')) {
        p->pos++;
        node.delimiter = parse_simple(p);
        if (node.delimiter == NULL) goto fail;
    }

    GroupNode *ptr = (GroupNode*) arena_alloc(&p->arena, sizeof(node));
    if (ptr == NULL) goto fail;

    *ptr = node;
    return (GroupNode*) ((char*) ptr - p->arena.ptr);

fail:
    p->pos = start_pos;
    return NULL;
}

SimpleNode *parse_simple(Parser *p) {
    Token *start_pos = p->pos;
    ptrdiff_t arena_offset = p->arena.pos - p->arena.ptr;

    SimpleNode node;
    node.type = T_SIMPLE;

    node.lookbehind = unknown_matches(p, '<');
    if (node.lookbehind) p->pos++;

    node.negate = unknown_matches(p, '~');
    if (node.negate) p->pos++;

    TokenType type = p->pos->type;
    if (type == T_STRING || type == T_RANGE || type == T_LEX_IDEN || type == T_PARSE_IDEN || unknown_matches(p, '.')) {
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

    return (SimpleNode*) ((char*) ptr - p->arena.ptr);

fail:
    p->pos = start_pos;
    // TODO: Clean up instead of stepping back alone.
    p->arena.pos = p->arena.ptr + arena_offset;
    return NULL;
}

MatchNode *parse_match(Parser *p) {
    Token *start_pos = p->pos;

    MatchNode node;
    node.type = T_MATCH;

    node.branch = parse_expr(p);
    if (node.branch == NULL) goto fail;

    MatchNode *ptr = (MatchNode*) arena_alloc(&p->arena, sizeof(MatchNode));
    if (ptr == NULL) goto fail;
    *ptr = node;

    return (MatchNode*) ((char*) ptr - p->arena.ptr);

fail:
    p->pos = start_pos;
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
    p->pos++;
    
    node.match = parse_match(p);
    if (node.match == NULL || !unknown_matches(p, ';')) goto fail;
    p->pos++;

    TokenDefNode *ptr = (TokenDefNode*) arena_alloc(&p->arena, sizeof(TokenDefNode));
    if (ptr == NULL) goto fail;
    *ptr = node;

    return (TokenDefNode*) ((char*) ptr - p->arena.ptr);

fail:
    p->pos = start_pos;
    return NULL;
}

ParseDefNode *parse_parsenode(Parser *p) {
    Token *start_pos = p->pos;
    ParseDefNode node;
    node.type = T_PARSEDEF;

    if (p->pos->type != T_PARSE_IDEN) goto fail;
    node.name = p->pos++;

    if (!unknown_matches(p, '=')) goto fail;
    p->pos++;

    node.match = parse_match(p);
    if (node.match == NULL || !unknown_matches(p, ';')) goto fail;
    p->pos++;

    ParseDefNode *ptr = (ParseDefNode*) arena_alloc(&p->arena, sizeof(ParseDefNode));
    if (ptr == NULL) goto fail;
    *ptr = node;

    return (ParseDefNode*) ((char*) ptr - p->arena.ptr);

fail:
    p->pos = start_pos;
    return NULL;
}

// TODO: Implement
void parse_grammar(Parser *p) {
    while (unknown_matches(p, ';')) p->pos++;
}