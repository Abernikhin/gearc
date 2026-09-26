
#ifndef _NODE_
#define _NODE_

#include "NodeTypes.h"
#include <stdbool.h>
#include <stddef.h>

struct type {
    bool is_reference;
    bool is_template;
    struct type* template;
    char* name;
};

void free_type(struct type* type);

/* Root of the AST hierarchy. */
struct node {
    enum GlobalNodes global_node;
};

/* Direct descendants of node. */
struct statement {
    struct node parent;
    enum StatementNodes statement_node;
};

/* Expressions are statements with expression-specific node types. */
struct expr {
    struct statement parent;
    enum ExprNodes expr_node;
};

/* Expression branches. */
struct constant {
    struct expr parent;
    enum ConstNodes const_node;
};

struct binary_op {
    struct expr parent;
    char* op;
    struct expr* left;
    struct expr* right;
};

struct unary_op {
    struct expr parent;
    char* op;
    struct expr* operand;
};

/* Constant branches. */
struct number {
    struct constant parent;
    int value;
};

struct string {
    struct constant parent;
    char* value;
};

struct id {
    struct constant parent;
    char* name;
};

/* Other statement branches. */
struct var {
    struct statement parent;
    struct type* type;
    char* name;
    struct expr* value;
};

struct assign {
    struct statement parent;
    struct id* id;
    struct expr* value;
};

struct call {
    struct statement parent;
    struct id* id;
    struct expr** args;
};

struct return_statement {
    struct statement parent;
    struct expr* value;
};

/* Function is a separate direct descendant of node. */
struct function {
    struct node parent;
    struct type* return_type;
    struct param {
        char* name;
        struct type* type;
    } *args;
    int param_count;
    struct statement** body;
    size_t body_count;
    char* name;
};

struct node* create_node(enum GlobalNodes);
struct statement* create_statement(struct node*, enum StatementNodes, ...);
struct expr* create_expr(struct statement*, enum ExprNodes, ...);
struct constant* create_constant(struct expr*, enum ConstNodes, ...);

void print_node(struct node*, int);
void free_node(struct node*);

#endif // _NODE_