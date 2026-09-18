
#include "parser/node.h"

#include <stdlib.h>

#define EXPR_SENTINEL ((enum StatementNodes)-1)

static void free_type(struct type* type) {
    if (type == NULL) {
        return;
    }

    free(type->name);
    free(type);
}

static void free_array_of_statements(struct statement** statements, size_t count) {
    if (statements == NULL) {
        return;
    }

    for (size_t i = 0; i < count; i++) {
        if (statements[i] != NULL) {
            free_node((struct node*)statements[i]);
        }
    }

    free(statements);
}

struct node* create_node(enum GlobalNodes global_node) {
    struct node* node = calloc(1, sizeof(*node));
    if (node == NULL) {
        return NULL;
    }

    node->global_node = global_node;
    return node;
}

struct statement* create_statement(struct node* parent, enum StatementNodes statement_node, ...) {
    struct statement* statement = calloc(1, sizeof(*statement));
    if (statement == NULL) {
        return NULL;
    }

    if (parent != NULL) {
        statement->parent = *parent;
    }

    statement->parent.global_node = Node_Statement;
    statement->statement_node = statement_node;

    return statement;
}

struct expr* create_expr(struct statement* parent, enum ExprNodes expr_node, ...) {
    struct expr* expr = calloc(1, sizeof(*expr));
    if (expr == NULL) {
        return NULL;
    }

    if (parent != NULL) {
        expr->parent = *parent;
    }

    expr->parent.parent.global_node = Node_Statement;
    expr->parent.statement_node = EXPR_SENTINEL;
    expr->expr_node = expr_node;

    return expr;
}

struct constant* create_constant(struct expr* parent, enum ConstNodes const_node, ...) {
    struct constant* constant = calloc(1, sizeof(*constant));
    if (constant == NULL) {
        return NULL;
    }

    if (parent != NULL) {
        constant->parent = *parent;
    }

    constant->parent.parent.parent.global_node = Node_Statement;
    constant->parent.parent.statement_node = EXPR_SENTINEL;
    constant->parent.expr_node = Node_Const;
    constant->const_node = const_node;

    return constant;
}

void free_node(struct node* node) {
    if (node == NULL) {
        return;
    }

    switch (node->global_node) {
        case Node_Function: {
            struct function* function = (struct function*)node;
            free_type(function->return_type);

            if (function->args != NULL) {
                for (size_t i = 0; i < 1; i++) {
                    free(function->args[i].name);
                    free_type(function->args[i].type);
                }
                free(function->args);
            }

            if (function->body != NULL) {
                for (size_t i = 0; i < 1; i++) {
                    if (function->body[i] != NULL) {
                        free_node((struct node*)function->body[i]);
                    }
                }
                free(function->body);
            }

            free(function->name);
            free(function);
            break;
        }
        case Node_Statement: {
            struct statement* statement = (struct statement*)node;

            if (statement->statement_node == EXPR_SENTINEL) {
                struct expr* expr = (struct expr*)node;

                switch (expr->expr_node) {
                    case Node_Binary_Op: {
                        struct binary_op* binary = (struct binary_op*)node;
                        free_node((struct node*)binary->left);
                        free_node((struct node*)binary->right);
                        free(binary->op);
                        free(binary);
                        break;
                    }
                    case Node_Unary_Op: {
                        struct unary_op* unary = (struct unary_op*)node;
                        free_node((struct node*)unary->operand);
                        free(unary->op);
                        free(unary);
                        break;
                    }
                    case Node_Const: {
                        struct constant* constant = (struct constant*)node;

                        switch (constant->const_node) {
                            case Node_Number: {
                                free(constant);
                                break;
                            }
                            case Node_String: {
                                struct string* text = (struct string*)node;
                                free(text->value);
                                free(text);
                                break;
                            }
                            case Node_Id: {
                                struct id* id = (struct id*)node;
                                free(id->name);
                                free(id);
                                break;
                            }
                            default:
                                free(constant);
                                break;
                        }
                        break;
                    }
                    default:
                        free(expr);
                        break;
                }
                break;
            }

            switch (statement->statement_node) {
                case Node_Var: {
                    struct var* variable = (struct var*)node;
                    free_type(variable->type);
                    free(variable->name);
                    free_node((struct node*)variable->value);
                    free(variable);
                    break;
                }
                case Node_Assign: {
                    struct assign* assignment = (struct assign*)node;
                    free(assignment->id->name);
                    free(assignment->id);
                    free_node((struct node*)assignment->value);
                    free(assignment);
                    break;
                }
                case Node_Call: {
                    struct call* call = (struct call*)node;
                    free(call->id->name);
                    free(call->id);
                    if (call->args != NULL) {
                        for (size_t i = 0; i < 1; i++) {
                            free_node((struct node*)call->args[i]);
                        }
                        free(call->args);
                    }
                    free(call);
                    break;
                }
                case Node_Return: {
                    struct return_statement* ret = (struct return_statement*)node;
                    free_node((struct node*)ret->value);
                    free(ret);
                    break;
                }
                default:
                    free(statement);
                    break;
            }
            break;
        }
        default:
            free(node);
            break;
    }
}


