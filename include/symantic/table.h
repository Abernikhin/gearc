
#ifndef _Table_
#define _Table_

#include "parser/node.h"

#include <stdbool.h>

struct symbol_var {
    char* name;
    struct type* type;
};

struct scope {
    struct symbol_var** vars;
    int varc;
};

struct symbol_fun
{
    char* name;
    struct type* type;
    struct type** args;
    int argc;
};


struct root {
    struct symbol_fun** funs;
    int func;
};

bool tpcmp(struct type*, struct type*);

struct symbol_var* var_new(char*, struct type*);
struct symbol_fun* fun_new(char*, struct type*, struct type**, int);

void print_var(struct symbol_var*);
void print_fun(struct symbol_fun*);

void free_var(struct symbol_var*);
void free_fun(struct symbol_fun*);

struct symbol_var* var_lookup(struct scope*, char*);
struct symbol_fun* fun_lookup(struct root*, char*);

#endif // _Table_
