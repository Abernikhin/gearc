
#ifndef _Table_
#define _Table_

#include "parser/node.h"

#include <stdbool.h>

struct symbol_var {
    char* name;
    struct type* type;
};

struct symbol_fun {
    char* name;
    struct type* return_type;
    struct type** arg_types;
    int arg_count;
};

struct scope {
    struct scope* parent;

    struct symbol_var* vars;
    int var_count;
    int var_capacity;

    struct symbol_fun* funcs;
    int fun_count;
    int fun_capacity;
};

bool typcmp(struct type* left, struct type* right);
bool scope_insert_var(struct scope* scope, const char* name, struct type* type);
struct symbol_var* scope_lookup_var(struct scope* scope, const char* name);
bool scope_insert_fun(struct scope* scope,
                     const char* name,
                     struct type* return_type,
                     struct type** arg_types,
                     int arg_count);
struct symbol_fun* scope_lookup_fun(struct scope* scope, const char* name);

#endif // _Table_
