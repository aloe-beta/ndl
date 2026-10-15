#ifndef NDG_H
#define NDG_H

#include <stddef.h>
#include <stdlib.h>

typedef unsigned int uint;

typedef enum {
    T_UNKNOWN,
    T_EOF,
    T_STRING,
    T_RANGE,
    T_LEX_IDEN,
    T_PARSE_IDEN
} TokenType;

static const char* TOKEN_NAMES[] = {
    [T_UNKNOWN]    = "<UNKNOWN>",
    [T_EOF]        = "<EOF>",
    [T_STRING]     = "STRING",
    [T_RANGE]      = "RANGE",
    [T_LEX_IDEN]   = "LEX_IDEN",
    [T_PARSE_IDEN] = "PARSE_IDEN"
};

typedef struct {
    TokenType type;
    const char* start;
    uint length;
    uint line;
} Token;

typedef struct {
    const char *source;
    const char *current;
    uint line;
} Lexer;

typedef enum {
    T_SIMPLE,
    T_EXPR,
    T_MATCH,
    T_TOKENDEF,
    T_PARSEDEF,
    T_STMT,
    T_GRAMMAR
} NodeType;

void init_lexer(Lexer *lexer, const char* source);

Token next_token(Lexer *lexer);

Token *tokenize(char *source, size_t size);

static const char* NODE_NAMES[] = {
    [T_SIMPLE]   = "simple",
    [T_EXPR]     = "expr",
    [T_MATCH]    = "match",
    [T_TOKENDEF] = "tokenDef",
    [T_PARSEDEF] = "parseDef",
    [T_STMT]     = "stmt",
    [T_GRAMMAR]  = "grammar"
};

typedef struct {
    char *ptr;
    char *pos;
    size_t size;
} Arena;

typedef struct {
    Arena arena;
    NodeType *first;
    Token *tokens;
} Parser;

#endif