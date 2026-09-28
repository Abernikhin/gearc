#ifndef _Convert_
#define _Convert_

#include "repr.h"
#include "parser/node.h"

struct converter {
    struct def** body;
    int count;
};

struct converter* create_converter();
void print_MIR(struct converter*);
void free_MIR(struct converter*);
void convert(struct converter*, struct node**);

#endif // _Convert_