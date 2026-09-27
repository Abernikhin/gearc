#include "symantic/table.h"

#include <stdlib.h>
#include <string.h>

bool typcmp(struct type* left, struct type* right) {
    if (left == right) {
        return true;
    }

    if (left == NULL || right == NULL) {
        return false;
    }

    if (left->is_reference != right->is_reference ||
        left->is_template != right->is_template) {
        return false;
    }

    if ((left->name == NULL && right->name != NULL) ||
        (left->name != NULL && right->name == NULL)) {
        return false;
    }

    if (left->name != NULL && strcmp(left->name, right->name) != 0) {
        return false;
    }

    if (left->template == NULL && right->template == NULL) {
        return true;
    }

    if (left->template == NULL || right->template == NULL) {
        return false;
    }

    return typcmp(left->template, right->template);
}

bool scope_insert_var(struct scope* scope, const char* name, struct type* type) {
    if (scope == NULL || name == NULL) {
        return false;
    }

    for (int i = 0; i < scope->var_count; i++) {
        if (strcmp(scope->vars[i].name, name) == 0) {
            return false;
        }
    }

    if (scope->var_count == scope->var_capacity) {
        int new_capacity = scope->var_capacity == 0 ? 4 : scope->var_capacity * 2;
        struct symbol_var* new_vars = realloc(scope->vars, sizeof(*new_vars) * new_capacity);
        if (new_vars == NULL) {
            return false;
        }

        scope->vars = new_vars;
        scope->var_capacity = new_capacity;
    }

    scope->vars[scope->var_count].name = strdup(name);
    if (scope->vars[scope->var_count].name == NULL) {
        return false;
    }

    scope->vars[scope->var_count].type = type;
    scope->var_count++;
    return true;
}

struct symbol_var* scope_lookup_var(struct scope* scope, const char* name) {
    for (struct scope* current = scope; current != NULL; current = current->parent) {
        for (int i = 0; i < current->var_count; i++) {
            if (strcmp(current->vars[i].name, name) == 0) {
                return &current->vars[i];
            }
        }
    }

    return NULL;
}

bool scope_insert_fun(struct scope* scope,
                     const char* name,
                     struct type* return_type,
                     struct type** arg_types,
                     int arg_count) {
    if (scope == NULL || name == NULL) {
        return false;
    }

    for (int i = 0; i < scope->fun_count; i++) {
        if (strcmp(scope->funcs[i].name, name) == 0) {
            return false;
        }
    }

    if (scope->fun_count == scope->fun_capacity) {
        int new_capacity = scope->fun_capacity == 0 ? 4 : scope->fun_capacity * 2;
        struct symbol_fun* new_funs = realloc(scope->funcs, sizeof(*new_funs) * new_capacity);
        if (new_funs == NULL) {
            return false;
        }

        scope->funcs = new_funs;
        scope->fun_capacity = new_capacity;
    }

    scope->funcs[scope->fun_count].name = strdup(name);
    if (scope->funcs[scope->fun_count].name == NULL) {
        return false;
    }

    scope->funcs[scope->fun_count].return_type = return_type;
    scope->funcs[scope->fun_count].arg_types = arg_types;
    scope->funcs[scope->fun_count].arg_count = arg_count;
    scope->fun_count++;
    return true;
}

struct symbol_fun* scope_lookup_fun(struct scope* scope, const char* name) {
    for (struct scope* current = scope; current != NULL; current = current->parent) {
        for (int i = 0; i < current->fun_count; i++) {
            if (strcmp(current->funcs[i].name, name) == 0) {
                return &current->funcs[i];
            }
        }
    }

    return NULL;
}
