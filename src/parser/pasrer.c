#include "lexer/token.h"
#include "parser/node.h"
#include "parser/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct parser* new_parser(struct token** tokens) {
    struct parser* p = (struct parser*)malloc(sizeof(struct parser));
    p->root = NULL;
    p->node_count = 0;
    p->tokens = tokens;
    p->current = *tokens;
    p->index = 0;
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
    for(int i = start; i < end; i++) {
        printf("%s ", self->tokens[i]->value);
    }
    printf("\n");
}

static void append(struct parser* self, struct node* node) {
    self->node_count++;
    self->root = (struct node**)realloc(self->root, sizeof(struct node*) * self->node_count);
    self->root[self->node_count - 1] = node;
}

static void advance(struct parser* self) {
    if(self->current->type != Token_EOF) {
        self->index++;
        self->current = self->tokens[self->index];
    }
}

static void error(struct parser* self) {
    get_current_line(self, self->index);
    for(int i = 1; i < self->current->column; i++) printf(" ");
    for(int i = 0; i < strlen(self->current->value); i++) printf("^");
    printf("\n");
    free_parser(self);
    exit(EXIT_FAILURE);
}

static void def(struct parser* self);

void parse(struct parser* self) {
    while(self->current->type != Token_EOF) {
        if(self->current->type == Token_Def_Kw) {
            def(self);
        } else {
            printf("\033[31m[error]\033[0m unknown statement\nat line %d\n", self->current->line);
            error(self);
        }
    }
}

static void def(struct parser* self) {
    advance(self);\
    struct function* node = (struct function*)create_node(Node_Function);
    if(self->current->type == Token_Id) {
        printf("\033[32m[info]\033[0m function definition: %s\n", self->current->value);
        node->name = strdup(self->current->value);
        advance(self);
    } else {
        printf("\033[31m[error]\033[0m unexpected name for function\nat line %d\n", self->current->line);
        error(self);
    }
}