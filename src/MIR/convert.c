#include "MIR/convert.h"
#include "parser/NodeTypes.h"
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct converter* create_converter() {
    struct converter* obj = malloc(sizeof(struct converter));
    if(obj == NULL) {
        return NULL;
    }

    obj->body = NULL;
    obj->count = 0;
    return obj;
}

void print_MIR(struct converter* self) {
    if (self == NULL) {
        printf("MIR <null>\n");
        return;
    }

    printf("MIR: definitions=%d\n", self->count);
    for (int i = 0; i < self->count; i++) {
        print_def(self->body[i]);
    }
}

void free_MIR(struct converter * self) {
    if (self == NULL) {
        return;
    }
    if(self->body != NULL) {
        for(int i = 0; i < self->count; i++) {
            free_def(self->body[i]);
        }
        free(self->body);
    }
    free(self);
}

static bool append_def(struct converter * self, struct def * obj) {
    if (self == NULL || obj == NULL || self->count < 0 || self->count == INT_MAX) {
        return false;
    }

    struct def** body = realloc(
        self->body,
        sizeof(*self->body) * (size_t)(self->count + 1));
    if (body == NULL) {
        return false;
    }

    self->body = body;
    self->body[self->count++] = obj;
    return true;
}

static bool emit_instruction(struct def* def,
                             enum Instructions opcode,
                             const char** operands,
                             size_t operand_count) {
    struct inst* inst = create_inst(opcode);
    if (inst == NULL) {
        return false;
    }

    for (size_t i = 0; i < operand_count; i++) {
        if (!append_op(inst, (char*)operands[i])) {
            free_inst(inst);
            return false;
        }
    }

    if (!append_inst(def, inst)) {
        free_inst(inst);
        return false;
    }
    return true;
}

static bool emit_move(struct def* def, const char* register_name, const char* value) {
    const char* operands[] = {register_name, value};
    return emit_instruction(def, Mov, operands, 2);
}

static char* simple_operand(struct expr* expr) {
    if (expr == NULL || expr->expr_node != Node_Const) {
        return NULL;
    }

    struct constant* constant = (struct constant*)expr;
    if (constant->const_node == Node_Id) {
        struct id* id = (struct id*)expr;
        return id->name != NULL ? strdup(id->name) : NULL;
    }
    if (constant->const_node == Node_Number) {
        char number[32];
        snprintf(number, sizeof(number), "%d", ((struct number*)expr)->value);
        return strdup(number);
    }
    if (constant->const_node == Node_String) {
        struct string* string = (struct string*)expr;
        return string->value != NULL ? strdup(string->value) : NULL;
    }

    return NULL;
}

static bool parse(struct def* def, struct expr* expr);

static bool parse_call(struct def* def, struct call* call) {
    if (call == NULL || call->id == NULL || call->id->name == NULL ||
        (call->arg_count > 0 && call->args == NULL)) {
        return false;
    }

    for (size_t i = 0; i < call->arg_count; i++) {
        char* operand = simple_operand(call->args[i]);
        if (operand != NULL) {
            const char* arg[] = {operand};
            bool emitted = emit_instruction(def, Arg, arg, 1);
            free(operand);
            if (!emitted) {
                return false;
            }
        } else {
            if (!parse(def, call->args[i])) {
                return false;
            }
            const char* accumulator[] = {"r1"};
            if (!emit_instruction(def, Arg, accumulator, 1)) {
                return false;
            }
        }
    }

    const char* name[] = {call->id->name};
    return emit_instruction(def, Call, name, 1);
}

static bool parse(struct def* def, struct expr* expr) {
    if (def == NULL || expr == NULL) {
        return false;
    }

    if (expr->expr_node == Node_Const) {
        char* operand = simple_operand(expr);
        if (operand == NULL) {
            return false;
        }
        bool emitted = emit_move(def, "r1", operand);
        free(operand);
        return emitted;
    }

    switch (expr->expr_node) {
        case Node_Binary_Op: {
            struct binary_op* binary = (struct binary_op*)expr;
            enum Instructions opcode;
            if (binary->op == NULL) {
                return false;
            }
            if (strcmp(binary->op, "+") == 0) {
                opcode = Add;
            } else if (strcmp(binary->op, "-") == 0) {
                opcode = Sub;
            } else if (strcmp(binary->op, "*") == 0) {
                opcode = Mul;
            } else if (strcmp(binary->op, "/") == 0) {
                opcode = Div;
            } else {
                return false;
            }

            char* right_operand = simple_operand(binary->right);
            if (right_operand != NULL) {
                bool valid = parse(def, binary->left);
                if (valid) {
                    valid = emit_move(def, "r2", right_operand);
                }
                if (valid) {
                    const char* operand[] = {"r2"};
                    valid = emit_instruction(def, opcode, operand, 1);
                }
                free(right_operand);
                return valid;
            }

            if (!parse(def, binary->left) ||
                !emit_instruction(def, Push, NULL, 0) ||
                !parse(def, binary->right) ||
                !emit_instruction(def, Pop, (const char*[]){"r2"}, 1)) {
                return false;
            }

            return emit_instruction(def, opcode, (const char*[]){"r2"}, 1);
        }
        case Node_Unary_Op: {
            struct unary_op* unary = (struct unary_op*)expr;
            if (unary->op == NULL || strcmp(unary->op, "-") != 0) {
                return false;
            }

            if (!parse(def, unary->operand) || !emit_move(def, "r2", "-1")) {
                return false;
            }
            return emit_instruction(def, Mul, (const char*[]){"r2"}, 1);
        }
        case Node_Call:
            return parse_call(def, (struct call*)expr);
        default:
            return false;
    }
}

void convert(struct converter * self, struct node ** tree, int count) {
    if (self == NULL || tree == NULL || count <= 0) {
        return;
    }

    for (int i = 0; i < count; i++) {
        if (tree[i] != NULL && tree[i]->global_node == Node_Function) {
            struct function* func = (struct function*)tree[i];
            struct def* def = create_def(func->name);
            if (def == NULL) {
                continue;
            }

            bool valid = true;
            if (func->param_count < 0 ||
                (func->param_count > 0 && func->args == NULL)) {
                valid = false;
            }

            for (int parameter = 0; valid && parameter < func->param_count; parameter++) {
                if (func->args[parameter].name == NULL) {
                    valid = false;
                    break;
                }
                const char* name[] = {func->args[parameter].name};
                valid = emit_instruction(def, New, name, 1);
            }

            for (int parameter = func->param_count - 1;
                 valid && parameter >= 0;
                 parameter--) {
                if (func->args[parameter].name == NULL) {
                    valid = false;
                    break;
                }
                const char* name[] = {func->args[parameter].name};
                valid = emit_instruction(def, Lod, name, 1);
            }

            for (size_t j = 0; j < func->body_count; j++) {
                if (!valid) {
                    break;
                }
                struct statement* statement = func->body[j];
                if (statement == NULL) {
                    continue;
                }
                switch (statement->statement_node) {
                    case Node_Var: {
                        struct var* variable = (struct var*)statement;
                        if (variable->name == NULL) {
                            valid = false;
                            break;
                        }

                        const char* name[] = {variable->name};
                        valid = emit_instruction(def, New, name, 1);
                        if (valid && variable->value != NULL) {
                            valid = parse(def, variable->value);
                            if (valid) {
                                valid = emit_instruction(def, Set, name, 1);
                            }
                        }
                        break;
                    }
                    case Node_Return: {
                        struct return_statement* ret = (struct return_statement*)statement;
                        valid = parse(def, ret->value);
                        if (valid) {
                            valid = emit_instruction(def, Ret, NULL, 0);
                        }
                        break;
                    }
                    case Node_Assign: {
                        struct assign* assignment = (struct assign*)statement;
                        if (assignment->id == NULL || assignment->id->name == NULL) {
                            valid = false;
                            break;
                        }
                        valid = parse(def, assignment->value);
                        if (valid) {
                            const char* name[] = {assignment->id->name};
                            valid = emit_instruction(def, Set, name, 1);
                        }
                        break;
                    }
                    default:
                        break;
                }
                if (!valid) {
                    fprintf(stderr, "[MIR error] failed to lower statement in function '%s'\n",
                            func->name != NULL ? func->name : "<unnamed>");
                    break;
                }
            }

            if (!valid) {
                free_def(def);
                continue;
            }
            if (!append_def(self, def)) {
                free_def(def);
            }
        }
    }
}
