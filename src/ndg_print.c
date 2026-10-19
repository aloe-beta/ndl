#include "ndg.h"
#include <stdio.h>

static void print_bool(char *name, bool value, uint indent) {
    printf("%*s%s: %s,\n", (indent+1)*2, "", name, value ? "true" : "false");
}

static void print_token(char *name, Token *token, uint indent) {
    printf("%*s%s: ", (indent+1)*2, "", name);
    if (token == NULL) {
        printf("NULL,\n");
        return;
    }
    printf("%s(%.*s),\n", TOKEN_NAMES[token->type], (int)token->length, token->start);
}

static void print_node(char *name, NodeType* type, uint indent) {
    printf("%*s%s: ", (indent+1)*2, "", name);
    if (type == NULL) {
        printf("NULL,\n");
        return;
    }
    print_field(type, indent+1);
}

static void print_expr(char *name, size_t* ptr, uint indent) {
    size_t count = *(ptr++);

    printf("%*s%s: [\n", (indent+1)*2, "", name);
    for (size_t i = 0; i < count; i++) {
        printf("%*s[\n", (indent+2)*2, "");

        size_t *array = *(size_t**) ptr;
        size_t count = *(array++);

        for (size_t j = 0; j < count; j++) {
            printf("%*s", (indent+3)*2, "");
            print_field((NodeType*) array[j], indent+3);
        }
        printf("%*s],\n", (indent+2)*2, "");
    }
    printf("%*s]\n", (indent+1)*2, "");
}

void print_field(NodeType* type, uint indent) {
    printf("%s {\n", NODE_NAMES[*type]);

    switch (*type) {
        case T_TOKEN: {
            TokenNode *node = (TokenNode*) type;
            print_token("token", node->token, indent);
            break;
        }
        case T_SIMPLE: {
            SimpleNode *node = (SimpleNode*) type;
            print_bool("lookahead", node->lookahead, indent);
            print_bool("lookbehind", node->lookbehind, indent);
            print_bool("negate", node->negate, indent);
            print_bool("flag", node->flag, indent);
            printf("%*squantity: <%c>,\n", (indent+1)*2, "", node->quantity);
            print_node("match", node->match, indent);
            print_token("iden", node->iden, indent);
            break;
        }
        case T_GROUP: {
            GroupNode *node = (GroupNode*) type;
            print_expr("branch", node->branch, indent);
            print_node("delimiter", (NodeType*) node->delimiter, indent);
            break;
        }
        case T_MATCH:{
            MatchNode *node = (MatchNode*) type;
            print_expr("branch", node->branch, indent);
            break;
        }
        case T_TOKENDEF: {
            TokenDefNode *node = (TokenDefNode*) type;
            print_bool("skip", node->skip, indent);
            print_token("name", node->name, indent);
            print_token("overrides", node->overrides, indent);
            print_node("match", (NodeType*) node->match, indent);
            break;
        }
        case T_PARSEDEF: {
            ParseDefNode *node = (ParseDefNode*) type;
            print_token("name", node->name, indent);
            print_node("match", (NodeType*) node->match, indent);
            break;
        }
        default: {
            break;
        }
    }

    printf("%*s}\n", indent*2, "");
}