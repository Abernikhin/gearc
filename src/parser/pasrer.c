#include "lexer/lexer.h"
#include "lexer/token.h"
#include "parser/node.h"
#include "parser/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct parser* new_parser(struct token** tokens, struct lexer* lexer) {
    struct parser* p = (struct parser*)malloc(sizeof(struct parser));
    p->root = NULL;
    p->node_count = 0;
    p->tokens = tokens;
    p->current = *tokens;
    p->lexer = lexer;
    p->index = 0;
    return p;
}

void free_parser(struct parser* self) {
    if (self == NULL) {
        return;
    }

    for (size_t i = 0; i < self->node_count; i++) {
        free_node(self->root[i]);
    }

    free(self->root);
    free(self);
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
    free_lexer(self->lexer);
    free_parser(self);
    exit(EXIT_FAILURE);
}

static void def(struct parser* self);

void parse(struct parser* self) {
    while(self->current->type != Token_EOF) {
        if(self->current->type == Token_Def_Kw) {
            def(self);
        } else {
            printf("\033[31m[error]\033[0m unknown statement at line %d\n", self->current->line);
            error(self);
        }
    }
}

static void add_param(struct function* func, char* name, struct type* type) {
    func->param_count++;
    func->args = (struct param*)realloc(func->args, sizeof(*func->args) * func->param_count);
    func->args[func->param_count - 1].name = strdup(name);
    func->args[func->param_count - 1].type = type;
}

static struct type* create_type(char* type, bool is_pointer) {
    if (type == NULL) {
        return NULL;
    }

    struct type* result = calloc(1, sizeof(*result));
    if (result == NULL) {
        return NULL;
    }

    result->name = strdup(type);
    if (result->name == NULL) {
        free(result);
        return NULL;
    }

    result->is_pointer = is_pointer;
    return result;
}

static void def(struct parser* self) {
    advance(self);
    struct function* node = (struct function*)create_node(Node_Function);

    if(self->current->type == Token_Id) {
        node->name = strdup(self->current->value);
        printf("\033[32m[info]\033[0m function definition: %s\n", node->name);
        advance(self);
    } else {
        printf("\033[31m[error]\033[0m unexpected name for function at line %d\n", self->current->line);
        error(self);
    }

    if(self->current->type == Token_Open) {
        advance(self);
        while(self->current->type != Token_Close && self->current->type != Token_EOF) {
            if(self->current->type == Token_Id) {
                printf("\033[32m[info]\033[0m function %s argument: %s\n", node->name, self->current->value);
                char* arg_name = self->current->value;
                struct type* arg_type = NULL;
                advance(self);
                if(self->current->type == Token_Colon) {
                    advance(self);
                    arg_type = create_type(self->current->value, false);
                    if (arg_type == NULL) {
                        error(self);
                    }
                    add_param(node, arg_name, arg_type);
                } else {
                    printf("\033[31m[error]\033[0m expected : after argument name at line %d\n", self->current->line);
                    free(arg_type);
                    error(self);
                }
                advance(self);
                if(self->current->type == Token_Comma) {
                    advance(self);
                    continue;
                } 
                if(self->current->type == Token_Close) continue;
                else {
                    printf("\033[31m[error]\033[0m expected , after argument type at line %d\n", self->current->line);
                    free(arg_type);
                    error(self);
                }
            } else {
                printf("\033[31m[error]\033[0m unexpected token %s in function arguments at line %d\n", self->current->value, self->current->line);
                error(self);
            }
        }
        if(self->current->type == Token_Close) {
            advance(self);
        } else {
            printf("\033[31m[error]\033[0m unexpected end of function %s arguments at line %d\n", node->name, self->current->line);
            error(self);
        }
    }

    if(self->current->type == Token_Colon) {
        advance(self);
        node->return_type = create_type(self->current->value, false);
        advance(self);
    } else {
        node->return_type = create_type("void", false);
    }

    if(self->current->type == Token_Begin) {
        advance(self);
        if(self->current->type == Token_End) {
            advance(self);
        }
    }

    append(self, (struct node*)node);

}