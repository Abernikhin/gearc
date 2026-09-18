#include "lexer/token.h"
#include <stdio.h>
#include <stdlib.h>

struct token* new_token(enum TokenType type, char* value, int line, int column) {
    struct token* self = malloc(sizeof(struct token));
    self->type = type;
    self->value = value;
    self->line = line;
    self->column = column;
    return self;
}

struct token* clone_token(struct token* self) {
    return new_token(self->type, self->value, self->line, self->column);
}

void print_token(struct token* self) {
    printf("Token: %d, Value: %s, Line: %d, Column: %d\n", self->type, self->value, self->line, self->column);
}

void free_token(struct token* self) {
    free(self->value);
    free(self);
}
