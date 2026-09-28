#include "MIR/repr.h"
#include <stdlib.h>
#include <string.h>

struct inst* create_inst(enum Instructions data) {
    struct inst* obj = malloc(sizeof(struct inst));
    if(obj == NULL) {
        return NULL;
    }
    obj->data = data;
    obj->op = NULL;

    return obj;
}

void print_inst(struct inst * self) {

}

void free_inst(struct inst * self) {
    if (self == NULL) {
        return;
    }
    if(self->op != NULL) {
        free(self->op);
    }
    free(self);
}

void append_op(struct inst * self, char * data) {
    if (self->op == NULL) {
        self->op = malloc(sizeof(char*));
        self->op[0] = strdup(data);
        return;
    }
    self->op = realloc(self->op, sizeof(char*) * 2);
    self->op[1] = strdup(data);
}

struct def* create_def(char* name) {
    struct def* obj = malloc(sizeof(struct def));
    if(obj == NULL) {
        return NULL;
    }
    obj->name = strdup(name);
    obj->body = NULL;
    obj->count = 0;
    return obj;
}

void print_def(struct def * self) {

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

void append_inst(struct def * self, struct inst * obj) {
    if (self->body == NULL) {
        self->body = malloc(sizeof(struct inst *));
        self->body[0] = obj;
        self->count++;
        return;
    }
    self->body = realloc(self->body, self->count++);
    self->body[self->count-1] = obj;
}
