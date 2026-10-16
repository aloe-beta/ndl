#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "ndg.h"

char *read_file(char *path, size_t *ret_size) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t read = fread(buffer, sizeof(char), size, file);
    if (read < size) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    if (ret_size != NULL) {
        *ret_size = size;
    }

    buffer[size] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return -1;
    }

    size_t size = 0;
    char *source = read_file(argv[1], &size);

    if (!source) {
        fprintf(stderr, "Error: failed to read %s\n", argv[1]);
        free(source);
        return -1;
    }

    Token *tokens = tokenize(source, size);
    // size_t i = 0;
    // while (1) {
    //     Token token = tokens[i];
    //     i++;
    //     printf("%s: <%.*s>\n", TOKEN_NAMES[token.type], (int)token.length, token.start);
    //     if (token.type == T_EOF) break;
    // }

    Parser parser;
    init_parser(&parser, tokens);
    
    size_t i = 0;
    while (1) {
        i++;
        TokenDefNode *tokenDef = parse_tokendef(&parser);
        if (tokenDef != NULL) {
            print_field(&parser, (size_t) tokenDef, 0);
            continue;
        }
        ParseDefNode *parseDef = parse_parsedef(&parser);
        if (parseDef != NULL) {
            print_field(&parser, (size_t) parseDef, 0);
            continue;
        }
        if (parser.pos->type == T_EOF) break;
        
        printf("UNCAUGHT TOKEN: %s(%.*s),\n", TOKEN_NAMES[parser.pos->type], (int)parser.pos->length, parser.pos->start);
        break;
    }

    free(source);
    return 0;
}