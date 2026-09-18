
#ifndef __TOKEN__
#define __TOKEN__

#include "TokenTypes.h"

struct token {

    enum TokenType type;
    char* value;
    int line;
    int column;

};

struct token* new_token(enum TokenType, char*, int, int);
struct token* clone_token(struct token*);
void print_token(struct token*);
void free_token(struct token*);

#endif // __TOKEN__
