
#include "parser/node.h"

#include <stdio.h>
#include <stdlib.h>

#define EXPR_SENTINEL ((enum StatementNodes)-1)

static void print_indent(int depth) {
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
}

static const char* statement_node_name(enum StatementNodes node) {
    switch (node) {
        case Node_Var:
            return "var";
        case Node_Return:
            return "return";
        case Node_Assign:
            return "assign";
        default:
            return "unknown_statement";
    }
}

static const char* expr_node_name(enum ExprNodes node) {
    switch (node) {
        case Node_Binary_Op:
            return "binary_op";
        case Node_Unary_Op:
            return "unary_op";
        case Node_Call:
            return "call";
        case Node_Const:
            return "const";
        default:
            return "unknown_expr";
    }
}

static const char* const_node_name(enum ConstNodes node) {
    switch (node) {
        case Node_Number:
            return "number";
        case Node_String:
            return "string";
        case Node_Id:
            return "id";
        default:
            return "unknown_const";
    }
}

void free_type(struct type* type) {
    if (type == NULL) {
        return;
    }

    free(type->name);
    free_type(type->template);
    free(type);
}

static void print_type(struct type* type) {
    if (type == NULL) {
        printf("<null>");
        return;
    }

    if (type->is_reference) {
        printf("&");
    }

    if (type->name != NULL) {
        printf("%s", type->name);
    } else {
        printf("<null>");
    }

    if (type->is_template && type->template != NULL) {
        printf("<");
        print_type(type->template);
        printf(">");
    }
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

static void print_expr_tree(struct expr* expr, int depth) {
    if (expr == NULL) {
        print_indent(depth);
        printf("(null)\n");
        return;
    }

    if (expr->expr_node == Node_Const) {
        struct constant* constant = (struct constant*)expr;
        print_indent(depth);
        switch (constant->const_node) {
            case Node_Number:
                printf("Const: %d\n", ((struct number*)expr)->value);
                break;
            case Node_String:
                printf("Const: %s\n", ((struct string*)expr)->value != NULL ? ((struct string*)expr)->value : "<null>");
                break;
            case Node_Id:
                printf("Const: %s\n", ((struct id*)expr)->name != NULL ? ((struct id*)expr)->name : "<null>");
                break;
            default:
                printf("Const: unknown\n");
                break;
        }
        return;
    }

    switch (expr->expr_node) {
        case Node_Binary_Op: {
            struct binary_op* binary = (struct binary_op*)expr;
            print_indent(depth);
            printf("%s\n", binary->op != NULL ? binary->op : "<null>");
            if (binary->left != NULL) {
                print_expr_tree(binary->left, depth + 1);
            }
            if (binary->right != NULL) {
                print_expr_tree(binary->right, depth + 1);
            }
            break;
        }
        case Node_Unary_Op: {
            struct unary_op* unary = (struct unary_op*)expr;
            print_indent(depth);
            printf("%s\n", unary->op != NULL ? unary->op : "<null>");
            if (unary->operand != NULL) {
                print_expr_tree(unary->operand, depth + 1);
            }
            break;
        }
        case Node_Call: {
            struct call* call = (struct call*)expr;
            print_indent(depth);
            printf("call %s(\n", call->id != NULL && call->id->name != NULL ? call->id->name : "<null>");
            if (call->args != NULL) {
                for (size_t i = 0; i < call->arg_count; i++) {
                    if (call->args[i] != NULL) {
                        print_expr_tree(call->args[i], depth + 1);
                    }
                }
            }
            print_indent(depth);
            printf(")\n");
            break;
        }
        default:
            print_indent(depth);
            printf("Expr: %s\n", expr_node_name(expr->expr_node));
            print_indent(depth + 1);
            printf("<unsupported expr>\n");
            break;
    }
}

struct node* create_node(enum GlobalNodes global_node) {
    size_t node_size = sizeof(struct node);

    switch (global_node) {
        case Node_Function:
            node_size = sizeof(struct function);
            break;
        case Node_Statement:
            node_size = sizeof(struct statement);
            break;
        default:
            break;
    }

    struct node* node = calloc(1, node_size);
    if (node == NULL) {
        return NULL;
    }

    node->global_node = global_node;

    return node;
}

struct statement* create_statement(struct node* parent, enum StatementNodes statement_node, ...) {
    size_t node_size = sizeof(struct statement);

    switch (statement_node) {
        case Node_Var:
            node_size = sizeof(struct var);
            break;
        case Node_Return:
            node_size = sizeof(struct return_statement);
            break;
        case Node_Assign:
            node_size = sizeof(struct assign);
            break;
        default:
            break;
    }

    struct statement* statement = calloc(1, node_size);
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
    size_t node_size = sizeof(struct expr);

    switch (expr_node) {
        case Node_Binary_Op:
            node_size = sizeof(struct binary_op);
            break;
        case Node_Unary_Op:
            node_size = sizeof(struct unary_op);
            break;
        case Node_Const:
            node_size = sizeof(struct constant);
            break;
        default:
            break;
    }

    struct expr* expr = calloc(1, node_size);
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
    size_t node_size = sizeof(struct constant);

    switch (const_node) {
        case Node_Number:
            node_size = sizeof(struct number);
            break;
        case Node_String:
            node_size = sizeof(struct string);
            break;
        case Node_Id:
            node_size = sizeof(struct id);
            break;
        default:
            break;
    }

    struct constant* constant = calloc(1, node_size);
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
                for (int i = 0; i < function->param_count; i++) {
                    free(function->args[i].name);
                    free_type(function->args[i].type);
                }
                free(function->args);
            }

            if (function->body != NULL) {
                for (size_t i = 0; i < function->body_count; i++) {
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
                    case Node_Call: {
                        struct call* call = (struct call*)node;
                        if (call->id != NULL) {
                            free(call->id->name);
                            free(call->id);
                        }
                        if (call->args != NULL) {
                            for (size_t i = 0; i < call->arg_count; i++) {
                                free_node((struct node*)call->args[i]);
                            }
                            free(call->args);
                        }
                        free(call);
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

void print_node(struct node* node, int depth) {
    if (node == NULL) {
        print_indent(depth);
        printf("(null)\n");
        return;
    }

    switch (node->global_node) {
        case Node_Function: {
            struct function* function = (struct function*)node;
            print_indent(depth);
            printf("Function: name=%s return_type=", function->name != NULL ? function->name : "<null>");
            print_type(function->return_type);
            printf(" args=%s body=%s\n",
                   function->args != NULL ? "<present>" : "<none>",
                   function->body != NULL ? "<present>" : "<none>");

            for (int i = 0; i < function->param_count; i++) {
                print_indent(depth + 1);
                printf("Parameter: name=%s type=",
                       function->args[i].name != NULL ? function->args[i].name : "<null>");
                print_type(function->args[i].type);
                printf("\n");
            }

            if (function->body != NULL) {
                for (size_t i = 0; i < function->body_count; i++) {
                    if (function->body[i] != NULL) {
                        print_node((struct node*)function->body[i], depth + 1);
                    }
                }
            }
            break;
        }
        case Node_Statement: {
            struct statement* statement = (struct statement*)node;

            if (statement->statement_node == EXPR_SENTINEL) {
                struct expr* expr = (struct expr*)node;
                print_indent(depth);
                printf("Expr: %s\n", expr_node_name(expr->expr_node));

                switch (expr->expr_node) {
                    case Node_Binary_Op: {
                        struct binary_op* binary = (struct binary_op*)node;
                        print_indent(depth + 1);
                        printf("op=%s\n", binary->op != NULL ? binary->op : "<null>");
                        if (binary->left != NULL) {
                            print_node((struct node*)binary->left, depth + 1);
                        }
                        if (binary->right != NULL) {
                            print_node((struct node*)binary->right, depth + 1);
                        }
                        break;
                    }
                    case Node_Unary_Op: {
                        struct unary_op* unary = (struct unary_op*)node;
                        print_indent(depth + 1);
                        printf("op=%s\n", unary->op != NULL ? unary->op : "<null>");
                        if (unary->operand != NULL) {
                            print_node((struct node*)unary->operand, depth + 1);
                        }
                        break;
                    }
                    case Node_Call: {
                        struct call* call = (struct call*)node;
                        print_indent(depth + 1);
                        printf("call=%s args=%zu\n",
                               call->id != NULL && call->id->name != NULL ? call->id->name : "<null>",
                               call->arg_count);
                        if (call->args != NULL) {
                            for (size_t i = 0; i < call->arg_count; i++) {
                                if (call->args[i] != NULL) {
                                    print_node((struct node*)call->args[i], depth + 2);
                                }
                            }
                        }
                        break;
                    }
                    case Node_Const: {
                        struct constant* constant = (struct constant*)node;
                        print_indent(depth + 1);
                        printf("Const: %s\n", const_node_name(constant->const_node));

                        switch (constant->const_node) {
                            case Node_Number: {
                                struct number* number = (struct number*)node;
                                print_indent(depth + 2);
                                printf("value=%d\n", number->value);
                                break;
                            }
                            case Node_String: {
                                struct string* text = (struct string*)node;
                                print_indent(depth + 2);
                                printf("value=%s\n", text->value != NULL ? text->value : "<null>");
                                break;
                            }
                            case Node_Id: {
                                struct id* id = (struct id*)node;
                                print_indent(depth + 2);
                                printf("name=%s\n", id->name != NULL ? id->name : "<null>");
                                break;
                            }
                            default:
                                break;
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            }

            switch (statement->statement_node) {
                case Node_Var: {
                    struct var* variable = (struct var*)node;
                    print_indent(depth);
                    printf("Statement: var %s",
                           variable->name != NULL ? variable->name : "<null>");
                    if (variable->type != NULL && variable->type->name != NULL) {
                        printf(": %s", variable->type->name);
                    }
                    printf(" = \n");
                    if (variable->value != NULL) {
                        print_expr_tree(variable->value, depth + 1);
                    }
                    break;
                }
                case Node_Assign: {
                    struct assign* assignment = (struct assign*)node;
                    print_indent(depth);
                    printf("Statement: assign %s =\n",
                           assignment->id != NULL && assignment->id->name != NULL ? assignment->id->name : "<null>");
                    if (assignment->value != NULL) {
                        print_expr_tree(assignment->value, depth + 1);
                    }
                    break;
                }
                case Node_Return: {
                    struct return_statement* ret = (struct return_statement*)node;
                    print_indent(depth);
                    printf("Statement: return\n");
                    if (ret->value != NULL) {
                        print_expr_tree(ret->value, depth + 1);
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            print_indent(depth);
            printf("Unknown node\n");
            break;
    }
}
