
#ifndef __Parser_h__
#define __Parser_h__

#include "node.h"
#include <stddef.h>

struct parser {
    struct node **root;
    size_t node_count;
    struct token** tokens;
    struct token* current;
    size_t index;
};

struct parser* new_parser(struct token**);
void free_parser(struct parser*);
void print_parser(struct parser*);
void parse(struct parser*);

#endif // __Parser_h__
