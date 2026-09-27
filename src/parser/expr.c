#include "expr.h"
#include "lexer/token.h"
#include "parser/node.h"

#include <stdlib.h>
#include <string.h>

static void advance_expr(struct parser* parser) {
	if (parser->current->type != Token_EOF) {
		parser->index++;
		parser->current = parser->tokens[parser->index];
	}
}

static void init_expr_parent(struct expr* expression, enum ExprNodes expr_node) {
	expression->parent.parent.global_node = Node_Statement;
	expression->parent.statement_node = (enum StatementNodes)-1;
	expression->expr_node = expr_node;
}

static struct expr* create_number(int value) {
	struct number* number = calloc(1, sizeof(*number));
	if (number == NULL) {
		return NULL;
	}

	init_expr_parent(&number->parent.parent, Node_Const);
	number->parent.const_node = Node_Number;
	number->value = value;
	return &number->parent.parent;
}

static struct expr* create_string(const char* value) {
	struct string* string = calloc(1, sizeof(*string));
	if (string == NULL) {
		return NULL;
	}

	string->value = strdup(value);
	if (string->value == NULL) {
		free(string);
		return NULL;
	}

	init_expr_parent(&string->parent.parent, Node_Const);
	string->parent.const_node = Node_String;
	return &string->parent.parent;
}

static struct expr* create_id(const char* name) {
	struct id* id = calloc(1, sizeof(*id));
	if (id == NULL) {
		return NULL;
	}

	id->name = strdup(name);
	if (id->name == NULL) {
		free(id);
		return NULL;
	}

	init_expr_parent(&id->parent.parent, Node_Const);
	id->parent.const_node = Node_Id;
	return &id->parent.parent;
}

static struct expr* create_call(struct id* id, struct expr** args, size_t arg_count) {
	struct call* call = calloc(1, sizeof(*call));
	if (call == NULL) {
		return NULL;
	}

	call->id = id;
	call->args = args;
	call->arg_count = arg_count;
	init_expr_parent(&call->parent, Node_Call);
	return &call->parent;
}

static struct expr* create_unary(const char* operator, struct expr* operand) {
	struct unary_op* unary = calloc(1, sizeof(*unary));
	if (unary == NULL) {
		return NULL;
	}

	unary->op = strdup(operator);
	if (unary->op == NULL) {
		free(unary);
		return NULL;
	}

	init_expr_parent(&unary->parent, Node_Unary_Op);
	unary->operand = operand;
	return &unary->parent;
}

static struct expr* create_binary(const char* operator, struct expr* left, struct expr* right) {
	struct binary_op* binary = calloc(1, sizeof(*binary));
	if (binary == NULL) {
		return NULL;
	}

	binary->op = strdup(operator);
	if (binary->op == NULL) {
		free(binary);
		return NULL;
	}

	init_expr_parent(&binary->parent, Node_Binary_Op);
	binary->left = left;
	binary->right = right;
	return &binary->parent;
}

struct expr* factor(struct parser* parser) {
	if (parser == NULL || parser->current == NULL) {
		return NULL;
	}

	struct token* token = parser->current;
	if (token->type == Token_Minus) {
		advance_expr(parser);
		struct expr* operand = factor(parser);
		if (operand == NULL) {
			return NULL;
		}
		return create_unary("-", operand);
	}

	if (token->type == Token_Open) {
		advance_expr(parser);
		struct expr* value = expr(parser);
		if (parser->current->type != Token_Close) {
			free_node((struct node*)value);
			return NULL;
		}
		advance_expr(parser);
		return value;
	}

	if (token->type == Token_Number) {
		int value = atoi(token->value);
		advance_expr(parser);
		return create_number(value);
	}

	if (token->type == Token_String) {
		struct expr* value = create_string(token->value);
		advance_expr(parser);
		return value;
	}

	if (token->type == Token_Id) {
		char* name = strdup(token->value);
		if (name == NULL) {
			return NULL;
		}

		advance_expr(parser);
		if (parser->current != NULL && parser->current->type == Token_Open) {
			advance_expr(parser);
			struct expr** args = NULL;
			size_t arg_count = 0;

			if (parser->current != NULL && parser->current->type != Token_Close) {
				while (1) {
					struct expr* arg = expr(parser);
					if (arg == NULL) {
						for (size_t i = 0; i < arg_count; i++) {
							free_node((struct node*)args[i]);
						}
						free(args);
						free(name);
						return NULL;
					}

					struct expr** new_args = realloc(args, sizeof(*new_args) * (arg_count + 1));
					if (new_args == NULL) {
						free_node((struct node*)arg);
						for (size_t i = 0; i < arg_count; i++) {
							free_node((struct node*)args[i]);
						}
						free(args);
						free(name);
						return NULL;
					}

					args = new_args;
					args[arg_count++] = arg;

					if (parser->current == NULL || parser->current->type != Token_Comma) {
						break;
					}
					advance_expr(parser);
				}
			}

			if (parser->current == NULL || parser->current->type != Token_Close) {
				for (size_t i = 0; i < arg_count; i++) {
					free_node((struct node*)args[i]);
				}
				free(args);
				free(name);
				return NULL;
			}

			advance_expr(parser);
			struct id* function_id = (struct id*)create_id(name);
			free(name);
			if (function_id == NULL) {
				for (size_t i = 0; i < arg_count; i++) {
					free_node((struct node*)args[i]);
				}
				free(args);
				return NULL;
			}

			return create_call(function_id, args, arg_count);
		}

		struct expr* value = create_id(name);
		free(name);
		return value;
	}

	return NULL;
}

struct expr* term(struct parser* parser) {
	struct expr* left = factor(parser);
	if (left == NULL) {
		return NULL;
	}

	while (parser->current->type == Token_Multiply || parser->current->type == Token_Divide) {
		const char* operator = parser->current->type == Token_Multiply ? "*" : "/";
		advance_expr(parser);

		struct expr* right = factor(parser);
		if (right == NULL) {
			free_node((struct node*)left);
			return NULL;
		}

		struct expr* result = create_binary(operator, left, right);
		if (result == NULL) {
			free_node((struct node*)left);
			free_node((struct node*)right);
			return NULL;
		}
		left = result;
	}

	return left;
}

struct expr* expr(struct parser* parser) {
	struct expr* left = term(parser);
	if (left == NULL) {
		return NULL;
	}

	while (parser->current->type == Token_Plus || parser->current->type == Token_Minus) {
		const char* operator = parser->current->type == Token_Plus ? "+" : "-";
		advance_expr(parser);

		struct expr* right = term(parser);
		if (right == NULL) {
			free_node((struct node*)left);
			return NULL;
		}

		struct expr* result = create_binary(operator, left, right);
		if (result == NULL) {
			free_node((struct node*)left);
			free_node((struct node*)right);
			return NULL;
		}
		left = result;
	}

	return left;
}

