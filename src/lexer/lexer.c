#include "lexer/lexer.h"
#include <ctype.h>
#include <stdio.h>
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
    self->current_word_column = 1;
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
    for (int i = 0; i < self->index; i++) {
        if (self->tokens[i] != NULL) {
            free_token(self->tokens[i]);
        }
    }
    free(self->tokens);
    free(self);
}

void print_lexer(struct lexer* self) {
    if (self == NULL) {
        return;
    }
    for (int i = 0; i < self->index; i++) {
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
    if (!(self->index < self->max)) {
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
}

static void append_char(char** buffer, char c) {
    if (buffer == NULL) {
        return;
    }

    size_t len = (*buffer == NULL) ? 0 : strlen(*buffer);
    char* new_buffer = realloc(*buffer, len + 2);
    if (new_buffer == NULL) {
        return;
    }

    *buffer = new_buffer;
    (*buffer)[len] = c;
    (*buffer)[len + 1] = '\0';
}

static void reset_buffer(char** buffer) {
    if (buffer == NULL || *buffer == NULL) {
        return;
    }
    (*buffer)[0] = '\0';
}

static enum TokenType identify_token_type(char* value);

static void emit_buffer(struct lexer* self, char** buffer) {
    if (self == NULL || buffer == NULL || *buffer == NULL || (*buffer)[0] == '\0') {
        return;
    }

    char* value = strdup(*buffer);
    if (value == NULL) {
        return;
    }

    lexer_add_token(self, new_token(identify_token_type(value), value, self->line, self->current_word_column));
    free(*buffer);
    *buffer = malloc(sizeof(char));
    if (*buffer != NULL) {
        (*buffer)[0] = '\0';
    }
    self->current_word_column = self->column;
}

static void emit_single_token(struct lexer* self, enum TokenType type, char c) {
    if (self == NULL) {
        return;
    }

    char* value = malloc(2);
    if (value == NULL) {
        return;
    }
    value[0] = c;
    value[1] = '\0';
    lexer_add_token(self, new_token(type, value, self->line, self->column));
}

static enum TokenType identify_token_type(char* value) {
    if (value == NULL || value[0] == '\0') {
        return Token_EOF;
    }
    if (strcmp(value, "def") == 0) {
        return Token_Def_Kw;
    } else if (strcmp(value, "var") == 0) {
        return Token_Var_Kw;
    } else if (strcmp(value, "return") == 0) {
        return Token_Return_Kw;
    } else if (isdigit((unsigned char)value[0])) {
        return Token_Number;
    } else if (isalpha((unsigned char)value[0]) || value[0] == '_') {
        return Token_Id;
    } else {
        return Token_EOF;
    }
}

void tokenize(struct lexer* self) {
    if (self == NULL) {
        return;
    }

    char* buffer = malloc(sizeof(char));
    if (buffer == NULL) {
        return;
    }
    buffer[0] = '\0';

    while (self->current != '\0') {
        if (self->current == '\n') {
            self->line++;
            self->column = 1;
        } else {
            self->column++;
        }

        if (self->current == ' ' || self->current == '\t' || self->current == '\n') {
            emit_buffer(self, &buffer);
            lexer_advance(self);
            continue;
        }

        if (self->current == '"') {
            lexer_advance(self);
            while (self->current != '"' && self->current != '\0') {
                append_char(&buffer, self->current);
                lexer_advance(self);
            }
            if (self->current == '"') {
                lexer_advance(self);
            }

            if (buffer[0] != '\0') {
                char* value = strdup(buffer);
                if (value != NULL) {
                    lexer_add_token(self, new_token(Token_String, value, self->line, self->column));
                }
            }
            free(buffer);
            buffer = malloc(sizeof(char));
            if (buffer != NULL) {
                buffer[0] = '\0';
            }
            continue;
        }

        if (self->current == '(') {
            emit_buffer(self, &buffer);
            emit_single_token(self, Token_Open, self->current);
            lexer_advance(self);
            continue;
        }
        if (self->current == ')') {
            emit_buffer(self, &buffer);
            emit_single_token(self, Token_Close, self->current);
            lexer_advance(self);
            continue;
        }
        if (self->current == '{') {
            emit_buffer(self, &buffer);
            emit_single_token(self, Token_Begin, self->current);
            lexer_advance(self);
            continue;
        }
        if (self->current == '}') {
            emit_buffer(self, &buffer);
            emit_single_token(self, Token_End, self->current);
            lexer_advance(self);
            continue;
        }
        if (self->current == ':') {
            emit_buffer(self, &buffer);
            emit_single_token(self, Token_Colon, self->current);
            lexer_advance(self);
            continue;
        }
        if (self->current == ';') {
            emit_buffer(self, &buffer);
            emit_single_token(self, Token_Semicolon, self->current);
            lexer_advance(self);
            continue;
        }
        if (self->current == ',') {
            emit_buffer(self, &buffer);
            emit_single_token(self, Token_Comma, self->current);
            lexer_advance(self);
            continue;
        }
        if (self->current == '=') {
            emit_buffer(self, &buffer);
            emit_single_token(self, Token_Assign, self->current);
            lexer_advance(self);
            continue;
        }
        if (self->current == '+') {
            emit_buffer(self, &buffer);
            emit_single_token(self, Token_Plus, self->current);
            lexer_advance(self);
            continue;
        }
        if (self->current == '-') {
            emit_buffer(self, &buffer);
            emit_single_token(self, Token_Minus, self->current);
            lexer_advance(self);
            continue;
        }
        if (self->current == '*') {
            emit_buffer(self, &buffer);
            emit_single_token(self, Token_Multiply, self->current);
            lexer_advance(self);
            continue;
        }
        if (self->current == '/') {
            emit_buffer(self, &buffer);
            emit_single_token(self, Token_Divide, self->current);
            lexer_advance(self);
            continue;
        }

        append_char(&buffer, self->current);
        lexer_advance(self);
    }

    emit_buffer(self, &buffer);

    char* eof_value = strdup("EOF");
    if (eof_value != NULL) {
        lexer_add_token(self, new_token(Token_EOF, eof_value, self->line, self->column));
    }

    free(buffer);
}
