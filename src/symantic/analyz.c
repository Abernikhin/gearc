#include "symantic/analyz.h"

#include <stdio.h>
#include <stdlib.h>

struct root* create_root(void) {
    return calloc(1, sizeof(struct root));
}

void print_root(struct root* root) {
    if (root == NULL) {
        printf("root <null>\n");
        return;
    }

    printf("root: functions=%d\n", root->func);
    for (int i = 0; i < root->func; i++) {
        if (root->funs[i] != NULL) {
            print_fun(root->funs[i]);
        }
    }
}

void free_root(struct root* root) {
    if (root == NULL) {
        return;
    }

    for (int i = 0; i < root->func; i++) {
        free_fun(root->funs[i]);
    }

    free(root->funs);
    free(root);
}

void analyz(struct node** nodes) {
    (void)nodes;
}
