#ifndef NDG_H
#define NDG_H

#include <stddef.h>

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

void init_lexer(Lexer *lexer, const char* source);

Token next_token(Lexer *lexer);

#endif