
#ifndef _Repr_
#define _Repr_

enum Instructions {
    Add,
    Sub,
    Mul,
    Div,
    New,
    Set,
    Call,
    Ret
};

struct inst {
    enum Instructions data;
    char** op;
};

struct inst* create_inst(enum Instructions);
void print_inst(struct inst*);
void free_inst(struct inst*);
void append_op(struct inst*, char*);

struct def {
    char* name;
    struct inst** body;
    int count;
};

struct def* create_def(char*);
void print_def(struct def*);
void free_def(struct def*);
void append_inst(struct def*, struct inst*);

#endif // _Repr_
