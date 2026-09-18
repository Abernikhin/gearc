#include "lexer/lexer.h"
#include <ctype.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

struct lexer* new_lexer(char* source) {
    struct lexer* self = malloc(sizeof(struct lexer));
    self->source = source;
    self->current = source[0];
    self->position = 0;
    self->line = 1;
    self->column = 1;
    self->tokens = malloc(sizeof(struct token*) * 1024);
    self->index = 0;
    self->max = 1024;
    for (int i = 0; i < self->max; i++) {
        self->tokens[i] = NULL;
    }
    return self;
}

void free_lexer(struct lexer* self) {
    if (self == NULL) {
        return;
    }
    for (int i = 0; i < 1024; i++) {
        free_token(self->tokens[i]);
    }
    free(self->tokens);
    free(self);
}

void print_lexer(struct lexer* self) {
    if (self == NULL) {
        return;
    }
    for (int i = 0; i < 1024; i++) {
        if (self->tokens[i] == NULL) {
            break;
        }
        print_token(self->tokens[i]);
    }
}

static void lexer_add_token(struct lexer* self, struct token* token) {
    if (self == NULL || token == NULL) {
        return;
    }
    if(!(self->index < self->max)) {
        self->max *= 2;
        self->tokens = realloc(self->tokens, sizeof(struct token*) * self->max);
    }
    self->tokens[self->index++] = token;
}

static void lexer_advance(struct lexer* self) {
    if (self == NULL) {
        return;
    }

    self->position++;
    self->current = self->source[self->position];
    if(self->current == '\n') {
        self->line++;
        self->column = 1;
    } else {
        self->column++;
    }
}

static void add_char(char * buffer, char c) {
    int len = 0;
    while (buffer[len] != '\0') {
        len++;
    }
    buffer = realloc(buffer, len + 2);
    buffer[len] = c;
    buffer[len + 1] = '\0';
}

static void reset_buffer(char * buffer) {
    buffer[0] = '\0';
}

static enum TokenType identify_token_type(char* value) {
    if (strcmp(value, "def") == 0) {
        return Token_Def_Kw;
    } else if (strcmp(value, "var") == 0) {
        return Token_Var_Kw;
    } else if (strcmp(value, "return") == 0) {
        return Token_Return_Kw;
    } else if (isdigit(value[0])) {
        return Token_Number;
    } else if (isalpha(value[0]) || value[0] == '_') {
        return Token_Id;
    } else {
        return Token_EOF; // Default case
    }
}

void tokenize(struct lexer* self) {
    if (self == NULL) {
        return;
    }

    char* buffer = malloc(sizeof(char));
    buffer[0] = '\0';

    while (self->current != '\0') {
        if (self->current == ' ' || self->current == '\t' || self->current == '\n') {
            lexer_add_token(self, new_token(identify_token_type(buffer), buffer, self->line, self->column));
            reset_buffer(buffer);
            lexer_advance(self);
            continue;
        }

        if (self->current == '"') {
            lexer_advance(self);
            while (self->current != '"' && self->current != '\0') {
                add_char(buffer, self->current);
                lexer_advance(self);
            }
            lexer_advance(self); // Skip closing quote
            struct token* token = new_token(Token_String, buffer, self->line, self->column);
            lexer_add_token(self, token);
            buffer[0] = '\0'; // Reset buffer
            continue;
        }

        lexer_advance(self);
    }
}
