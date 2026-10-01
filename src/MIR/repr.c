#include "MIR/repr.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct inst* create_inst(enum Instructions data) {
    struct inst* obj = malloc(sizeof(struct inst));
    if(obj == NULL) {
        return NULL;
    }
    obj->data = data;
    obj->op = NULL;
    obj->op_count = 0;

    return obj;
}

static const char* instruction_name(enum Instructions instruction) {
    switch (instruction) {
        case Add: return "Add";
        case Sub: return "Sub";
        case Mul: return "Mul";
        case Div: return "Div";
        case New: return "New";
        case Set: return "Set";
        case Mov: return "Mov";
        case Arg: return "Arg";
        case Lod: return "Lod";
        case Push: return "Push";
        case Pop: return "Pop";
        case Call: return "Call";
        case Ret: return "Ret";
        default: return "unknown";
    }
}

void print_inst(struct inst* self) {
    if (self == NULL) {
        printf("<null instruction>\n");
        return;
    }

    printf("    %s", instruction_name(self->data));
    if (self->data == Mov && self->op_count >= 2) {
        printf(" %s, %s",
               self->op[0] != NULL ? self->op[0] : "<null>",
               self->op[1] != NULL ? self->op[1] : "<null>");
        for (size_t i = 2; i < self->op_count; i++) {
            printf(" %s", self->op[i] != NULL ? self->op[i] : "<null>");
        }
    } else {
        for (size_t i = 0; i < self->op_count; i++) {
            printf(" %s", self->op[i] != NULL ? self->op[i] : "<null>");
        }
    }
    printf("\n");
}

void free_inst(struct inst * self) {
    if (self == NULL) {
        return;
    }
    for (size_t i = 0; i < self->op_count; i++) {
        free(self->op[i]);
    }
    free(self->op);
    free(self);
}

bool append_op(struct inst * self, char * data) {
    if (self == NULL || data == NULL || self->op_count == (size_t)-1) {
        return false;
    }

    char* operand = strdup(data);
    if (operand == NULL) {
        return false;
    }

    char** operands = realloc(self->op, sizeof(*self->op) * (self->op_count + 1));
    if (operands == NULL) {
        free(operand);
        return false;
    }

    self->op = operands;
    self->op[self->op_count++] = operand;
    return true;
}

struct def* create_def(char* name) {
    if (name == NULL) {
        return NULL;
    }

    struct def* obj = malloc(sizeof(struct def));
    if(obj == NULL) {
        return NULL;
    }
    obj->name = strdup(name);
    if (obj->name == NULL) {
        free(obj);
        return NULL;
    }
    obj->body = NULL;
    obj->count = 0;
    return obj;
}

void print_def(struct def* self) {
    if (self == NULL) {
        printf("  <null definition>\n");
        return;
    }

    printf("  def %s {\n", self->name != NULL ? self->name : "<unnamed>");
    for (int i = 0; i < self->count; i++) {
        print_inst(self->body[i]);
    }
    printf("  }\n");
}

void free_def(struct def * self) {
    if (self == NULL) {
        return;
    }
    if(self->body != NULL) {
        for(int i = 0; i < self->count; i++) {
            free_inst(self->body[i]);
        }
        free(self->body);
    }
    free(self->name);
    free(self);
}

bool append_inst(struct def* self, struct inst* obj) {
    if (self == NULL || obj == NULL || self->count < 0 || self->count == INT_MAX) {
        return false;
    }

    struct inst** body = realloc(self->body, sizeof(*self->body) * (size_t)(self->count + 1));
    if (body == NULL) {
        return false;
    }

    self->body = body;
    self->body[self->count++] = obj;
    return true;
}
