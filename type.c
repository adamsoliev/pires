#include "luan.h"

struct Type *ty_int = &(struct Type){TY_INT};

bool is_integer(struct Type *ty) { return ty->kind == TY_INT; }

struct Type *pointer_to(struct Type *base) {
    struct Type *ty = calloc(1, sizeof(struct Type));
    ty->kind = TY_PTR;
    ty->base = base;
    return ty;
}

void add_type(struct Node *node) {
    if (!node || node->ty) return;

    add_type(node->lhs);
    add_type(node->rhs);
    add_type(node->cond);
    add_type(node->then);
    add_type(node->els);
    add_type(node->init);
    add_type(node->inc);

    for (struct Node *n = node->body; n; n = n->next) add_type(n);

    switch (node->kind) {
        case ND_ADD:
        case ND_SUB:
        case ND_MUL:
        case ND_DIV:
        case ND_NEG:
        case ND_ASSIGN:
            node->ty = node->lhs->ty;
            return;
        case ND_EQ:
        case ND_NE:
        case ND_LT:
        case ND_LE:
        case ND_VAR:
        case ND_NUM:
            node->ty = ty_int;
            return;
        case ND_ADDR:
            node->ty = pointer_to(node->lhs->ty);
            return;
        case ND_DEREF:
            if (node->lhs->ty->kind == TY_PTR)
                node->ty = node->lhs->ty->base;
            else
                node->ty = ty_int;
            return;
        default:
            break;
    }
}