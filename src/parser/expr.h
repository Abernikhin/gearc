
#ifndef _Expr_
#define _Expr_

#include "parser/node.h"
#include "parser/parser.h"

struct expr* factor(struct parser*);
struct expr* term(struct parser*);
struct expr* expr(struct parser*);

#endif // _Expr_
