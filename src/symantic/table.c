#include "symantic/table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool tpcmp(struct type* left, struct type* right) {
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

    return tpcmp(left->template, right->template);
}

struct symbol_var* var_new(char* name, struct type* type) {
    if (name == NULL) {
        return NULL;
    }

    struct symbol_var* var = calloc(1, sizeof(*var));
    if (var == NULL) {
        return NULL;
    }

    var->name = strdup(name);
    if (var->name == NULL) {
        free(var);
        return NULL;
    }

    var->type = type;
    return var;
}

struct symbol_fun* fun_new(char* name, struct type* type, struct type** args, int argc) {
    if (name == NULL || argc < 0 || (argc > 0 && args == NULL)) {
        return NULL;
    }

    struct symbol_fun* fun = calloc(1, sizeof(*fun));
    if (fun == NULL) {
        return NULL;
    }

    fun->name = strdup(name);
    if (fun->name == NULL) {
        free(fun);
        return NULL;
    }

    if (argc > 0) {
        fun->args = malloc(sizeof(*fun->args) * (size_t)argc);
        if (fun->args == NULL) {
            free(fun->name);
            free(fun);
            return NULL;
        }

        memcpy(fun->args, args, sizeof(*fun->args) * (size_t)argc);
    }

    fun->type = type;
    fun->argc = argc;
    return fun;
}

static void print_type(const struct type* type) {
    if (type == NULL) {
        printf("<null>");
        return;
    }

    if (type->is_reference) {
        printf("&");
    }

    printf("%s", type->name != NULL ? type->name : "<null>");

    if (type->is_template) {
        printf("<");
        print_type(type->template);
        printf(">");
    }
}

void print_var(struct symbol_var* var) {
    if (var == NULL) {
        printf("var <null>\n");
        return;
    }

    printf("var %s: ", var->name != NULL ? var->name : "<null>");
    print_type(var->type);
    printf("\n");
}

void print_fun(struct symbol_fun* fun) {
    if (fun == NULL) {
        printf("fun <null>\n");
        return;
    }

    printf("fun %s(", fun->name != NULL ? fun->name : "<null>");
    for (int i = 0; i < fun->argc; i++) {
        if (i > 0) {
            printf(", ");
        }
        print_type(fun->args[i]);
    }
    printf("): ");
    print_type(fun->type);
    printf("\n");
}

void free_var(struct symbol_var* var) {
    if (var == NULL) {
        return;
    }

    free(var->name);
    free_type(var->type);
    free(var);
}

void free_fun(struct symbol_fun* fun) {
    if (fun == NULL) {
        return;
    }

    free(fun->name);
    free_type(fun->type);
    for (int i = 0; i < fun->argc; i++) {
        free_type(fun->args[i]);
    }
    free(fun->args);
    free(fun);
}

struct symbol_var* var_lookup(struct scope* scope, char* name) {
    if (name == NULL) {
        return NULL;
    }

    for (int i = 0; scope != NULL && i < scope->varc; i++) {
        if (scope->vars[i] != NULL &&
            scope->vars[i]->name != NULL &&
            strcmp(scope->vars[i]->name, name) == 0) {
            return scope->vars[i];
        }
    }

    return NULL;
}

struct symbol_fun* fun_lookup(struct scope* scope, char* name) {
    if (name == NULL) {
        return NULL;
    }

    for (int i = 0; scope != NULL && i < scope->func; i++) {
        if (scope->funs[i] != NULL &&
            scope->funs[i]->name != NULL &&
            strcmp(scope->funs[i]->name, name) == 0) {
            return scope->funs[i];
        }
    }

    return NULL;
}


