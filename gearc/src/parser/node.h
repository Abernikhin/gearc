#ifndef NODE_H
#define NODE_H

#include <stddef.h>

enum GlobalNodes {
    Node_Function,
    Node_Statement,
};

enum StatementNodes {
    Node_Var,
    Node_Return,
    Node_Assign,
    Node_Call,
};

enum ExprNodes {
    Node_Binary_Op,
    Node_Unary_Op,
    Node_Const,
};

enum ConstNodes {
    Node_Number,
    Node_String,
    Node_Id,
};

struct type {
    char* name;
    int is_pointer;
};

struct node {
    enum GlobalNodes global_node;
};

struct statement {
    struct node parent;
    enum StatementNodes statement_node;
};

struct expr {
    struct statement parent;
    enum ExprNodes expr_node;
};

struct constant {
    struct expr parent;
    enum ConstNodes const_node;
};

struct function {
    struct node parent;
    char* name;
    struct type* return_type;
    struct {
        char* name;
        struct type* type;
    }* args;
    size_t param_count;
    struct statement** body;
    size_t body_count;
};

struct binary_op {
    struct expr parent;
    struct expr* left;
    struct expr* right;
    char* op;
};

struct unary_op {
    struct expr parent;
    struct expr* operand;
    char* op;
};

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

struct var {
    struct statement parent;
    char* name;
    struct type* type;
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

struct node* create_node(enum GlobalNodes global_node);
struct statement* create_statement(struct node* parent, enum StatementNodes statement_node, ...);
struct expr* create_expr(struct statement* parent, enum ExprNodes expr_node, ...);
struct constant* create_constant(struct expr* parent, enum ConstNodes const_node, ...);
void free_node(struct node* node);
void print_node(struct node* node, int depth);

#endif // NODE_H