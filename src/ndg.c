#include "ndg.h"

void init_lexer(Lexer *l, const char* source) {
    l->source = source;
    l->current = source;
    l->line = 1;
}

static char peek(Lexer *l) {
    return *l->current;
}

static char peek_at(Lexer *l, int o) {
    return *(l->current + o);
}

// Note: skip won't increment the line number; use with caution.
static void skip(Lexer *l) {
    l->current++;
}

static void skip_by(Lexer *l, int o) {
    l->current += o;
}

static char next(Lexer *l) {
    char c = *l->current;
    l->current++;

    if (c == '\n') l->line++;

    return c;
}

static char is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static char is_alphanumeric(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

// A lot of this probably breaks when the file ends abruptly.
Token next_token(Lexer *l) {
    // Skipped tokens
skip:
    // WS
    while (is_whitespace(peek(l))) next(l);
    // COMMENT
    if (peek(l) == '/') {
        char c = peek_at(l, 1);

        if (c == '/') {
            l->current += 2;

            while (peek(l) != '\n' && peek(l) != '\0') skip(l);
            if (peek(l) == '\n') {
                next(l);
            }

            goto skip;
        }

        if (c == '*') {
            l->current += 2;

            while (peek(l) != '*' && peek_at(l, 1) != '/') next(l);

            l->current += 2;
            goto skip;
        }
    }

    // Captured tokens
    TokenType token = T_UNKNOWN;
    const char *start = l->current;
    const uint line = l->line;

    char c = next(l);

    // STRING and RANGE
    if (c == '"' || c == '\'' || c == '[') {
        if (c == '[') c = ']';

        while (1) {
            char c2 = peek(l);

            if (c2 == '\\') {
                skip(l);
                next(l);
                continue;
            }

            if (c2 == c) {
                skip(l);
                break;
            }

            next(l);
        }

        token = c == ']' ? T_RANGE : T_STRING;
        goto end;
    }

    // LEX_IDEN and PARSE_IDEN
    char lex = c >= 'A' && c <= 'Z';
    if (lex || (c >= 'a' && c <= 'z')) {
        while (is_alphanumeric(peek(l))) skip(l);

        token = lex ? T_LEX_IDEN : T_PARSE_IDEN;
        goto end;
    }

    // EOF
    if (c == '\0') token = T_EOF;

end:
    return (Token){ token, start, (uint)(l->current - start), line };
}