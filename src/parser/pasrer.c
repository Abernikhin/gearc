#include "lexer/TokenTypes.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "parser/node.h"
#include "parser/parser.h"
#include "expr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct parser* new_parser(struct token** tokens, struct lexer* lexer) {
    if (tokens == NULL || *tokens == NULL) {
        return NULL;
    }

    struct parser* p = calloc(1, sizeof(*p));
    if (p == NULL) {
        return NULL;
    }

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

    free_node((struct node*)self->pending_statement);
    free_node((struct node*)self->pending_function);

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

static bool append(struct parser* self, struct node* node) {
    struct node** new_root = realloc(self->root, sizeof(*new_root) * (self->node_count + 1));
    if (new_root == NULL) {
        return false;
    }

    self->root = new_root;
    self->root[self->node_count++] = node;
    return true;
}

static void advance(struct parser* self) {
    if(self->current->type != Token_EOF) {
        self->index++;
        self->current = self->tokens[self->index];
    }
}

static void error(struct parser* self) {
    get_current_line(self, self->index);
    for(int i = 1; i < self->current->column-1; i++) printf(" ");
    /*for(int i = 0; i < strlen(self->current->value); i++)*/ printf("^");
    printf("\n");
    longjmp(self->error_jump, 1);
}

static void def(struct parser* self);

bool parse(struct parser* self) {
    if (self == NULL || self->current == NULL) {
        return false;
    }

    if (setjmp(self->error_jump) != 0) {
        return false;
    }

    while(self->current->type != Token_EOF) {
        if(self->current->type == Token_Def_Kw) {
            def(self);
        } else {
            printf("\033[31m[error]\033[0m unknown statement at line %d\n", self->current->line);
            error(self);
        }
    }

    return true;
}

static bool add_param(struct function* func, char* name, struct type* type) {
    char* param_name = strdup(name);
    if (param_name == NULL) {
        return false;
    }

    struct param* args = realloc(func->args, sizeof(*args) * (size_t)(func->param_count + 1));
    if (args == NULL) {
        free(param_name);
        return false;
    }

    func->args = args;
    func->args[func->param_count].name = param_name;
    func->args[func->param_count].type = type;
    func->param_count++;
    return true;
}

static bool append_to_body(struct function* self, struct statement* child) {
    if (self == NULL || child == NULL) {
        return false;
    }

    size_t new_count = self->body_count + 1;
    struct statement** new_body = realloc(self->body, sizeof(*new_body) * new_count);
    if (new_body == NULL) {
        return false;
    }

    self->body = new_body;
    self->body[self->body_count] = child;
    self->body_count = new_count;
    return true;
}

static struct type* create_type(char* type, bool is_reference, bool is_template, struct type* template) {
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

    result->is_reference = is_reference;
    result->is_template = is_template;
    result->template = template;
    return result;
}

static struct type* parse_type(struct parser* self) {
    if (self == NULL || self->current == NULL) {
        return NULL;
    }

    bool is_reference = false;
    if (self->current->type == Token_And) {
        is_reference = true;
        advance(self);
    }

    if (self->current == NULL || self->current->type != Token_Id) {
        return NULL;
    }

    char* name = strdup(self->current->value);
    if (name == NULL) {
        return NULL;
    }

    advance(self);

    if (self->current != NULL && self->current->type == Token_Multiply) {
        free(name);
        return NULL;
    }

    struct type* type = create_type(name, is_reference, false, NULL);
    free(name);
    if (type == NULL) {
        return NULL;
    }

    if (self->current != NULL && self->current->type == Token_Less) {
        advance(self);
        struct type* template = parse_type(self);
        if (template == NULL || self->current == NULL || self->current->type != Token_Greater) {
            free_type(type);
            if (template != NULL) {
                free_type(template);
            }
            return NULL;
        }

        advance(self);
        type->is_template = true;
        type->template = template;
    }

    return type;
}

static struct statement* Return(struct parser* self);
static struct statement* Var(struct parser* self);
static bool append_to_body(struct function* self, struct statement* child);

static void def(struct parser* self) {
    advance(self);
    struct function* node = (struct function*)create_node(Node_Function);
    if (node == NULL) {
        printf("\033[31m[error]\033[0m could not allocate function node\n");
        error(self);
    }
    self->pending_function = node;

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
                    arg_type = parse_type(self);
                    if (arg_type == NULL) {
                        printf("\033[31m[error]\033[0m invalid argument type at line %d\n", self->current->line);
                        error(self);
                    }
                    if (!add_param(node, arg_name, arg_type)) {
                        free_type(arg_type);
                        error(self);
                    }
                } else {
                    printf("\033[31m[error]\033[0m expected : after argument name at line %d\n", self->current->line);
                    free_type(arg_type);
                    error(self);
                }
                if(self->current->type == Token_Comma) {
                    advance(self);
                    continue;
                } 
                if(self->current->type == Token_Close) continue;
                else {
                    printf("\033[31m[error]\033[0m expected , after argument type at line %d\n", self->current->line);
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
        node->return_type = parse_type(self);
        if (node->return_type == NULL) {
            printf("\033[31m[error]\033[0m invalid return type at line %d\n", self->current->line);
            error(self);
        }
    } else {
        node->return_type = create_type("void", false, false, NULL);
    }

    if(self->current->type == Token_Begin) {
        advance(self);
        while (self->current->type != Token_End && self->current->type != Token_EOF)
        {
            struct statement* child = NULL;
            if(self->current->type == Token_Return_Kw) {
                child = Return(self);
            } else if (self->current->type == Token_Var_Kw) {
                child = Var(self);
            } else {
                printf("\033[31m[error]\033[0m unexpected statement at line %d\n", self->current->line);
                error(self);
            }

            if (child == NULL || !append_to_body(node, child)) {
                printf("\033[31m[error]\033[0m could not add statement to function body\n");
                error(self);
            }
            self->pending_statement = NULL;

            if(self->current->type == Token_Semicolon) {
                advance(self);
            } else {
                printf("\033[31m[error]\033[0m at end of statemant must be ; at line %d\n", self->current->line);
                error(self);
            }
        }
        
        if(self->current->type == Token_End) {
            advance(self);
        } else {
            printf("\033[31m[error]\033[0m unexpected end of function %s at line %d\n", node->name, self->current->line);
            error(self);
        }
    }

    if (!append(self, (struct node*)node)) {
        printf("\033[31m[error]\033[0m could not add function to parser root\n");
        error(self);
    }
    self->pending_function = NULL;

}

static struct statement* Var(struct parser* self) {
    advance(self);

    struct var* variable = (struct var*)create_statement(NULL, Node_Var);
    if (variable == NULL) {
        error(self);
    }
    self->pending_statement = (struct statement*)variable;

    if (self->current->type != Token_Id) {
        printf("\033[31m[error]\033[0m expected variable name at line %d\n", self->current->line);
        error(self);
    }

    variable->name = strdup(self->current->value);
    advance(self);

    if (self->current->type == Token_Colon) {
        advance(self);
        variable->type = parse_type(self);
        if (variable->type == NULL) {
            printf("\033[31m[error]\033[0m invalid variable type at line %d\n", self->current->line);
            error(self);
        }
    } else {
        variable->type = create_type("void", false, false, NULL);
    }

    if (self->current->type == Token_Assign) {
        advance(self);
        variable->value = expr(self);
        if (variable->value == NULL) {
            printf("\033[31m[error]\033[0m invalid variable initializer at line %d\n", self->current->line);
            error(self);
        }
    }

    return (struct statement*)variable;
}

static struct statement* Return(struct parser* self) {
    advance(self);
    struct return_statement* ret = (struct return_statement*)create_statement(NULL, Node_Return);
    if (ret == NULL) {
        error(self);
    }
    self->pending_statement = (struct statement*)ret;
    ret->value = expr(self);
    if (ret->value == NULL) {
        printf("\033[31m[error]\033[0m invalid return value at line %d\n", self->current->line);
        error(self);
    }
    return (struct statement*)ret;
}
