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

static int align_to(int n, int align) {
    return (n + align - 1) / align * align;
}

static void gen_addr(struct Node *node) {
    if (node->kind == ND_VAR) {
        printf("  addi a0, fp, %d\n", node->var->offset);
        return;
    }
    error("Not an lvalue");
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
        case ND_VAR:
            gen_addr(node);
            printf("  ld a0, 0(a0)\n");
            return;
        case ND_ASSIGN:
            gen_addr(node->lhs);
            push();
            gen_expr(node->rhs);
            pop("a1");
            printf("  sd a0, 0(a1)\n");
            return;
        default:
            break;
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
        default:
            break;
    }
    error("Invalid expression");
};

static void get_stmt(struct Node *node) {
    switch (node->kind) {
        case ND_RETURN:
            gen_expr(node->lhs);
            printf("   j .L.return\n");
            return;
        case ND_EXPR_STMT:
            gen_expr(node->lhs);
            return;
        default:
            break;
    }
    error("Invalid statement");
};

static void assign_lvar_offsets(struct Function *prog) {
    int offset = 0;
    for (struct Obj *var = prog->locals; var; var = var->next) {
        offset += 8;
        var->offset = -offset;
    }
    prog->stack_size = align_to(offset, 16);
}

void codegen(struct Function *prog) {
    assign_lvar_offsets(prog);

    printf("  .globl main\n");
    printf("main:\n");

    printf("  addi sp, sp, -8\n");
    printf("  sd fp, 0(sp)\n");
    printf("  mv fp, sp\n");
    printf("  addi sp, sp, -%d\n", prog->stack_size);

    for (struct Node *n = prog->body; n; n = n->next) {
        get_stmt(n);
        assert(depth == 0);
    }

    printf(".L.return:\n");
    printf("  mv sp, fp\n");
    printf("  ld fp, 0(sp)\n");
    printf("  addi sp, sp, 8\n");

    printf("  ret\n");
}
