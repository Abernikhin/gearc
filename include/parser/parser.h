
#ifndef __Parser_h__
#define __Parser_h__

#include "node.h"
#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>

struct parser {
    struct node **root;
    size_t node_count;
    struct token** tokens;
    struct token* current;
    size_t index;
    struct lexer* lexer;
    jmp_buf error_jump;
    struct function* pending_function;
    struct statement* pending_statement;
};

struct parser* new_parser(struct token**, struct lexer*);
void free_parser(struct parser*);
void print_parser(struct parser*);
bool parse(struct parser*);

#endif // __Parser_h__
