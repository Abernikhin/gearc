
#ifndef __LEXER__
#define __LEXER__

#include "token.h"

struct lexer {
    char* source;
    char current;
    int position;
    int line;
    int column;
    int current_word_column;
    struct token** tokens;
    int index;
    int max;
};

struct lexer* new_lexer(char*);
void free_lexer(struct lexer*);
void print_lexer(struct lexer*);
void tokenize(struct lexer*);


#endif // __LEXER__
