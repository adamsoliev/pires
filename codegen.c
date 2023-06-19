#include <assert.h>
#include <stdio.h>

#include "pires.h"

static int depth;
static char *argreg[] = {"a0", "a1", "a2", "a3", "a4", "a5"};
static struct Function *current_fn;

static void gen_expr(struct Node *node);

static int count(void) {
    // FIXME: this isn't safe
    static int i = 1;
    return i++;
}

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
    switch (node->kind) {
        case ND_VAR:
            // printf("  addi a0, fp, %d\n", node->var->offset);
            printf("  sw a5, %d(s0)\n", node->var->offset);
            return;
        case ND_DEREF:
            gen_expr(node->lhs);
            return;
        default:
            break;
    }
    error_tok(node->token, "Not an lvalue");
}

static void gen_expr(struct Node *node) {
    switch (node->kind) {
        case ND_NUM:
            printf("  li a5, %d\n", node->val);
            return;
        case ND_NEG:
            gen_expr(node->lhs);
            printf("  neg a0, a0\n");
            return;
        case ND_VAR:
            // gen_addr(node);
            printf("  ld a5, %d(s0)\n", node->var->offset);
            return;
        case ND_DEREF:
            gen_expr(node->lhs);
            printf("  ld a0, 0(a0)\n");
            return;
        case ND_ADDR:
            gen_addr(node->lhs);
            return;
        case ND_ASSIGN:
            gen_expr(node->rhs);
            gen_addr(node->lhs);
            return;
        case ND_FUNCALL: {
            int nargs = 0;
            for (struct Node *arg = node->args; arg; arg = arg->next) {
                gen_expr(arg);
                push();
                nargs++;
            }
            for (int i = nargs - 1; i >= 0; i--) {
                pop(argreg[i]);
            }
            printf("   call %s\n", node->funcname);
            return;
        }
        default:
            break;
    }

    // gen_expr(node->rhs);
    // push();
    // gen_expr(node->lhs);
    // pop("a1");

    switch (node->kind) {
        case ND_ADD:
            gen_expr(node->lhs);
            printf("  mv a4, a5\n");
            gen_expr(node->rhs);
            printf("  add a5, a5, a4\n");
            return;
        case ND_SUB:
            gen_expr(node->lhs);
            printf("  mv a4, a5\n");
            gen_expr(node->rhs);
            printf("  sub a5, a4, a5\n");
            return;
        case ND_MUL:
            gen_expr(node->lhs);
            printf("  mv a4, a5\n");
            gen_expr(node->rhs);
            printf("  mul a5, a4, a5\n");
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
    error_tok(node->token, "Invalid expression");
};

static void gen_stmt(struct Node *node) {
    switch (node->kind) {
        case ND_IF: {
            int c = count();
            gen_expr(node->cond);
            printf("  beqz a0, .L.else.%d\n", c);
            gen_stmt(node->then);
            printf("  j .L.end.%d\n", c);
            printf(".L.else.%d:\n", c);
            if (node->els) gen_stmt(node->els);
            printf(".L.end.%d:\n", c);
            return;
        }
        case ND_FOR: {
            int c = count();
            if (node->init) gen_stmt(node->init);
            printf(".L.begin.%d:\n", c);
            if (node->cond) {
                gen_expr(node->cond);
                printf("  beqz a0, .L.end.%d\n", c);
            }
            gen_stmt(node->then);
            if (node->inc) gen_expr(node->inc);
            printf("  j .L.begin.%d\n", c);
            printf(".L.end.%d:\n", c);
            return;
        }
        case ND_BLOCK:
            for (struct Node *n = node->body; n; n = n->next) {
                gen_stmt(n);
            }
            return;
        case ND_RETURN:
            gen_expr(node->lhs);
            return;
        case ND_EXPR_STMT:
            gen_expr(node->lhs);
            return;
        default:
            break;
    }
    error_tok(node->token, "Invalid statement");
};

static void assign_lvar_offsets(struct Function *prog) {
    for (struct Function *fn = prog; fn; fn = fn->next) {
        int offset = 8;  // caller's frame pointer
        for (struct Obj *var = fn->locals; var; var = var->next) {
            offset += 4;
            var->offset = -offset;
        }
        offset += 16;
        fn->stack_size = align_to(offset, 16);
    }
}

void codegen(struct Function *prog) {
    assign_lvar_offsets(prog);
    for (struct Function *fn = prog; fn; fn = fn->next) {
        printf("  .globl %s\n", fn->name);
        printf("%s:\n", fn->name);
        current_fn = fn;

        // prologue
        printf("  addi sp, sp, %d\n", -fn->stack_size);
        // printf("  sd ra, 8(sp)\n");
        printf("  sd s0, %d(sp)\n", fn->stack_size - 8);
        printf("  addi s0, sp, %d\n", fn->stack_size);

        // save passed-by-register arguments to the stack
        int i = 0;
        for (struct Obj *var = fn->params; var; var = var->next) {
            printf("  sd %s, %d(fp)\n", argreg[i++], var->offset);
        }

        gen_stmt(fn->body);
        assert(depth == 0);

        // epilogue
        printf("  mv a0,a5\n");
        // printf("  ld ra, 8(sp)\n");
        printf("  ld s0,%d(sp)\n", fn->stack_size - 8);
        printf("  addi sp, sp, %d\n", fn->stack_size);
        printf("  jr ra\n");
    }
}
