#include "lexer/token.h"
#include "parser/parser.h"
#include <stdio.h>
#include <stdlib.h>

struct parser* new_parser(struct token** tokens) {
    struct parser* p = (struct parser*)malloc(sizeof(struct parser));
    p->root = NULL;
    p->node_count = 0;
    p->tokens = tokens;
    p->current = *tokens;
    return p;
}

void free_parser(struct parser* self) {
    if (self) {
        free(self);
    }
}

void print_parser(struct parser* self) {
    for(size_t i = 0; i < self->node_count; i++) {
        print_node(self->root[i], 0);
    }
}

static void get_current_line(struct parser* self, int index) {
    int start = index;
    while(self->tokens[start]->line == self->tokens[index]->line && start > 0) {
        start--;
    }
    int end = index;
    while(self->tokens[end]->line == self->tokens[index]->line && self->tokens[end]->type != Token_EOF) {
        end++;
    }
    for(int i = start + 1; i < end; i++) {
        printf("%s ", self->tokens[i]->value);
    }
}

static void append(struct parser* self, struct node* node) {
    self->node_count++;
    self->root = (struct node**)realloc(self->root, sizeof(struct node*) * self->node_count);
    self->root[self->node_count - 1] = node;
}

void parse(struct parser* self) {
    while(self->current->type != Token_EOF) {
        
    }
}
