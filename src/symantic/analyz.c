#include "symantic/analyz.h"
#include "parser/NodeTypes.h"
#include "symantic/table.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct root* create_root(void) {
    return calloc(1, sizeof(struct root));
}

static void append(struct root* root, struct symbol_fun* fun) {
    if (fun == NULL) {
        return;
    }

    if (root == NULL || root->func < 0 || root->func == INT_MAX) {
        free_fun(fun);
        return;
    }

    struct symbol_fun** funs = realloc(
        root->funs,
        sizeof(*root->funs) * (size_t)(root->func + 1));
    if (funs == NULL) {
        free_fun(fun);
        return;
    }

    root->funs = funs;
    root->funs[root->func] = fun;
    root->func++;
}

void print_root(struct root* root) {
    if (root == NULL) {
        printf("root <null>\n");
        return;
    }

    printf("root: functions=%d\n", root->func);
    for (int i = 0; i < root->func; i++) {
        if (root->funs[i] != NULL) {
            print_fun(root->funs[i]);
        }
    }
}

void free_root(struct root* root) {
    if (root == NULL) {
        return;
    }

    for (int i = 0; i < root->func; i++) {
        free_fun(root->funs[i]);
    }

    free(root->funs);
    free(root);
}

#define func(node) ((struct function*) node)

static struct type* copy_type(const struct type* source) {
    if (source == NULL) {
        return NULL;
    }

    struct type* copy = calloc(1, sizeof(*copy));
    if (copy == NULL) {
        return NULL;
    }

    copy->is_reference = source->is_reference;
    copy->is_template = source->is_template;

    if (source->name != NULL) {
        copy->name = strdup(source->name);
        if (copy->name == NULL) {
            free(copy);
            return NULL;
        }
    }

    if (source->template != NULL) {
        copy->template = copy_type(source->template);
        if (copy->template == NULL) {
            free_type(copy);
            return NULL;
        }
    }

    return copy;
}

static void free_type_array(struct type** types, int count) {
    if (types == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        free_type(types[i]);
    }
    free(types);
}

static void free_scope(struct scope* scope) {
    for (int i = 0; i < scope->varc; i++) {
        free_var(scope->vars[i]);
    }
    free(scope->vars);
}

static bool scope_add_var(struct scope* scope, struct symbol_var* var) {
    if (scope == NULL || var == NULL || var_lookup(scope, var->name) != NULL ||
        scope->varc == INT_MAX) {
        return false;
    }

    struct symbol_var** vars = realloc(
        scope->vars,
        sizeof(*scope->vars) * (size_t)(scope->varc + 1));
    if (vars == NULL) {
        return false;
    }

    scope->vars = vars;
    scope->vars[scope->varc++] = var;
    return true;
}

static bool type_is_void(const struct type* type) {
    return type != NULL && !type->is_reference && !type->is_template &&
        type->name != NULL && strcmp(type->name, "void") == 0;
}

static bool infer_expression_type(struct root* root,
                                  struct scope* scope,
                                  struct expr* expression,
                                  struct type** inferred_type) {
    static struct type integer_type = {.name = "i32"};
    static struct type character_reference_type = {.is_reference = true, .name = "c8"};

    *inferred_type = NULL;
    if (expression == NULL) {
        return true;
    }

    switch (expression->expr_node) {
        case Node_Const: {
            struct constant* constant = (struct constant*)expression;
            switch (constant->const_node) {
                case Node_Number:
                    *inferred_type = &integer_type;
                    return true;
                case Node_String:
                    *inferred_type = &character_reference_type;
                    return true;
                case Node_Id: {
                    struct id* id = (struct id*)expression;
                    struct symbol_var* var = id->name != NULL
                        ? var_lookup(scope, id->name)
                        : NULL;
                    if (var == NULL) {
                        fprintf(stderr,
                                "[semantic error] use of undeclared variable '%s'\n",
                                id->name != NULL ? id->name : "<invalid>");
                        return false;
                    }
                    *inferred_type = var->type;
                    return true;
                }
                default:
                    fprintf(stderr, "[semantic error] unsupported constant node\n");
                    return false;
            }
        }
        case Node_Binary_Op: {
            struct binary_op* binary = (struct binary_op*)expression;
            struct type* left_type = NULL;
            struct type* right_type = NULL;
            bool left_ok = infer_expression_type(root, scope, binary->left, &left_type);
            bool right_ok = infer_expression_type(root, scope, binary->right, &right_type);
            if (!left_ok || !right_ok) {
                return false;
            }
            if (left_type == NULL || right_type == NULL || !tpcmp(left_type, right_type)) {
                fprintf(stderr, "[semantic error] incompatible operand types for '%s'\n",
                        binary->op != NULL ? binary->op : "?");
                return false;
            }
            *inferred_type = left_type;
            return true;
        }
        case Node_Unary_Op: {
            struct unary_op* unary = (struct unary_op*)expression;
            return infer_expression_type(root, scope, unary->operand, inferred_type);
        }
        case Node_Call: {
            struct call* call = (struct call*)expression;
            const char* name = call->id != NULL && call->id->name != NULL
                ? call->id->name
                : "<invalid>";
            struct symbol_fun* function = call->id != NULL && call->id->name != NULL
                ? fun_lookup(root, call->id->name)
                : NULL;
            bool valid = true;

            if (function == NULL) {
                fprintf(stderr, "[semantic error] unknown function '%s'\n", name);
                valid = false;
            } else if (call->arg_count != (size_t)function->argc) {
                fprintf(stderr,
                        "[semantic error] function '%s' expects %d arguments, got %zu\n",
                        name,
                        function->argc,
                        call->arg_count);
                valid = false;
            }

            for (size_t i = 0; i < call->arg_count; i++) {
                struct type* argument_type = NULL;
                if (!infer_expression_type(root, scope, call->args[i], &argument_type)) {
                    valid = false;
                } else if (function != NULL && i < (size_t)function->argc &&
                           (argument_type == NULL || function->args[i] == NULL ||
                            !tpcmp(argument_type, function->args[i]))) {
                    fprintf(stderr,
                            "[semantic error] argument %zu of '%s' has an incompatible type\n",
                            i + 1,
                            name);
                    valid = false;
                }
            }

            if (valid && function != NULL) {
                *inferred_type = function->type;
            }
            return valid;
        }
        default:
            fprintf(stderr, "[semantic error] unsupported expression node\n");
            return false;
    }
}

static bool check_function_body(struct root* root, struct function* function) {
    struct scope scope = {0};
    bool valid = true;

    for (int i = 0; i < function->param_count; i++) {
        struct type* type = copy_type(function->args[i].type);
        struct symbol_var* var = type != NULL
            ? var_new(function->args[i].name, type)
            : NULL;
        if (var == NULL) {
            free_type(type);
        }
        if (var == NULL || !scope_add_var(&scope, var)) {
            free_var(var);
            fprintf(stderr, "[semantic error] invalid or duplicate parameter in '%s'\n",
                    function->name != NULL ? function->name : "<unnamed>");
            valid = false;
        }
    }

    for (size_t i = 0; i < function->body_count; i++) {
        struct statement* statement = function->body[i];
        if (statement == NULL) {
            valid = false;
            continue;
        }

        switch (statement->statement_node) {
            case Node_Var: {
                struct var* variable = (struct var*)statement;
                struct type* value_type = NULL;
                bool value_valid = infer_expression_type(root, &scope, variable->value, &value_type);
                if (!value_valid) {
                    valid = false;
                }

                struct type* declared_type = variable->type;
                if (type_is_void(declared_type)) {
                    declared_type = value_type;
                } else if (value_valid && value_type != NULL &&
                           !tpcmp(declared_type, value_type)) {
                    fprintf(stderr, "[semantic error] initializer type mismatch for '%s'\n",
                            variable->name != NULL ? variable->name : "<unnamed>");
                    valid = false;
                    break;
                }

                if (declared_type == NULL || variable->name == NULL ||
                    (value_type == NULL && type_is_void(variable->type))) {
                    fprintf(stderr, "[semantic error] variable '%s' has no usable type\n",
                            variable->name != NULL ? variable->name : "<unnamed>");
                    valid = false;
                    break;
                }

                struct type* stored_type = copy_type(declared_type);
                struct symbol_var* symbol = stored_type != NULL
                    ? var_new(variable->name, stored_type)
                    : NULL;
                if (symbol == NULL) {
                    free_type(stored_type);
                }
                if (symbol == NULL || !scope_add_var(&scope, symbol)) {
                    free_var(symbol);
                    fprintf(stderr, "[semantic error] duplicate or invalid variable '%s'\n",
                            variable->name);
                    valid = false;
                }
                break;
            }
            case Node_Return: {
                struct return_statement* ret = (struct return_statement*)statement;
                struct type* return_type = NULL;
                if (!infer_expression_type(root, &scope, ret->value, &return_type)) {
                    valid = false;
                } else if (return_type == NULL || function->return_type == NULL ||
                           !tpcmp(return_type, function->return_type)) {
                    fprintf(stderr, "[semantic error] return type mismatch in '%s'\n",
                            function->name != NULL ? function->name : "<unnamed>");
                    valid = false;
                }
                break;
            }
            case Node_Assign: {
                struct assign* assignment = (struct assign*)statement;
                struct symbol_var* target = assignment->id != NULL && assignment->id->name != NULL
                    ? var_lookup(&scope, assignment->id->name)
                    : NULL;
                struct type* value_type = NULL;
                if (target == NULL) {
                    fprintf(stderr, "[semantic error] assignment to undeclared variable\n");
                    valid = false;
                }
                if (!infer_expression_type(root, &scope, assignment->value, &value_type)) {
                    valid = false;
                } else if (target != NULL &&
                           (value_type == NULL || !tpcmp(target->type, value_type))) {
                    fprintf(stderr, "[semantic error] assignment type mismatch for '%s'\n",
                            target->name);
                    valid = false;
                }
                break;
            }
            default:
                fprintf(stderr, "[semantic error] unsupported statement in function '%s'\n",
                        function->name != NULL ? function->name : "<unnamed>");
                valid = false;
                break;
        }
    }

    free_scope(&scope);
    return valid;
}

bool analyz(struct node** nodes, int nodec) {
    if (nodes == NULL || nodec <= 0) {
        return nodes != NULL && nodec == 0;
    }

    struct root* root = create_root();
    if (root == NULL) {
        fprintf(stderr, "[semantic error] could not allocate symbol table\n");
        return false;
    }

    bool valid = true;

    for (int i = 0; i < nodec; i++) {
        if (nodes[i] == NULL || nodes[i]->global_node != Node_Function) {
            continue;
        }

        struct function* function = func(nodes[i]);
        if (function->name == NULL) {
            fprintf(stderr, "[semantic error] function has no name\n");
            valid = false;
            continue;
        }

        if (fun_lookup(root, function->name) != NULL) {
            fprintf(stderr, "[semantic error] duplicate function '%s'\n", function->name);
            valid = false;
            continue;
        }

        if (function->param_count < 0 ||
            (function->param_count > 0 && function->args == NULL)) {
            fprintf(stderr, "[semantic error] invalid parameter list in '%s'\n", function->name);
            valid = false;
            continue;
        }

        struct type** types = NULL;
        if (function->param_count > 0) {
            types = calloc((size_t)function->param_count, sizeof(*types));
            if (types == NULL) {
                fprintf(stderr, "[semantic error] out of memory while analyzing '%s'\n", function->name);
                valid = false;
                continue;
            }
        }

        bool types_ok = true;
        for (int a = 0; a < function->param_count; a++) {
            types[a] = copy_type(function->args[a].type);
            if (function->args[a].type == NULL || types[a] == NULL) {
                types_ok = false;
                break;
            }
        }

        struct type* return_type = copy_type(function->return_type);
        if (function->return_type == NULL || return_type == NULL || !types_ok) {
            fprintf(stderr, "[semantic error] invalid or unavailable type in '%s'\n", function->name);
            free_type_array(types, function->param_count);
            free_type(return_type);
            valid = false;
            continue;
        }

        struct symbol_fun* symbol = fun_new(
            function->name,
            return_type,
            types,
            function->param_count);
        if (symbol == NULL) {
            fprintf(stderr, "[semantic error] out of memory while registering '%s'\n", function->name);
            free_type_array(types, function->param_count);
            free_type(return_type);
            valid = false;
            continue;
        }

        free(types);
        int previous_count = root->func;
        append(root, symbol);
        if (root->func == previous_count) {
            fprintf(stderr, "[semantic error] out of memory while registering '%s'\n", function->name);
            valid = false;
        }
    }

    for (int i = 0; i < nodec; i++) {
        if (nodes[i] != NULL && nodes[i]->global_node == Node_Function &&
            !check_function_body(root, func(nodes[i]))) {
            valid = false;
        }
    }

    free_root(root);
    return valid;
}
