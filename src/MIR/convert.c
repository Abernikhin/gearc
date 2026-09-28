#include "MIR/convert.h"
#include <stdlib.h>


struct converter* create_converter() {
    struct converter* obj = malloc(sizeof(struct converter));
    if(obj == NULL) {
        return NULL;
    }

    obj->body = NULL;
    obj->count = 0;
    return obj;
}

void print_MIR(struct converter * self) {

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

static void append_def(struct converter * self, struct def * obj) {
    if (self->body == NULL) {
        self->body = malloc(sizeof(struct def *));
        self->body[0] = obj;
        self->count++;
        return;
    }
    self->body = realloc(self->body, self->count++);
    self->body[self->count-1] = obj;
}

