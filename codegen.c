#include <assert.h>
#include <stdio.h>

#include "luan.h"

// code generator
static int depth;

static void push(void) {
    printf("  addi sp, sp, -8\n");
    printf("  sd a0, 0(sp)\n");
    depth++;
}

static void pop(char *reg) {
    printf("  ld %s, 0(sp)\n", reg);
    printf("  addi sp, sp, 8\n");
    depth--;
}

static void gen_expr(struct Node *node) {
    switch (node->kind) {
        case ND_NUM:
            printf("  li a0, %d\n", node->val);
            return;
        case ND_NEG:
            gen_expr(node->lhs);
            printf("  neg a0, a0\n");
            return;
    }

    gen_expr(node->rhs);
    push();
    gen_expr(node->lhs);
    pop("a1");

    switch (node->kind) {
        case ND_ADD:
            printf("  add a0, a0, a1\n");
            return;
        case ND_SUB:
            printf("  sub a0, a0, a1\n");
            return;
        case ND_MUL:
            printf("  mul a0, a0, a1\n");
            return;
        case ND_DIV:
            printf("  div a0, a0, a1\n");
            return;
        case ND_EQ:
        case ND_NE:
            printf("    xor a0, a0, a1\n");
            if (node->kind == ND_EQ)
                printf("    seqz a0, a0\n");
            else
                printf("    snez a0, a0\n");
            return;
        case ND_LT:
            printf("    slt a0, a0, a1\n");
            return;
        case ND_LE:
            printf("    slt a0, a1, a0\n");
            printf("    xori a0, a0, 1\n");
            return;
    }
    error("Invalid expression");
};

static void get_stmt(struct Node *node) {
    if (node->kind == ND_EXPR_STMT) {
        gen_expr(node->lhs);
        return;
    }
    error("Invalid statement");
};

void codegen(struct Node *node) {
    printf("  .globl main\n");
    printf("main:\n");

    for (struct Node *n = node; n; n = n->next) {
        get_stmt(n);
        assert(depth == 0);
    }
    printf("  ret\n");
}
