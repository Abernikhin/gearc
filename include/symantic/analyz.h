
#ifndef _Analyz_
#define _Analyz_

#include "parser/node.h"
#include "table.h"

struct root* create_root();
void print_root(struct root*);
void free_root(struct root*);

bool analyz(struct node**, int);

#endif // _Analyz_
