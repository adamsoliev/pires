
#include "luan.h"

static struct Node *expr(struct Token **rest, struct Token *tok);
static struct Node *equality(struct Token **rest, struct Token *tok);
static struct Node *relational(struct Token **rest, struct Token *tok);
static struct Node *add(struct Token **rest, struct Token *tok);
static struct Node *mul(struct Token **rest, struct Token *tok);
static struct Node *unary(struct Token **rest, struct Token *tok);
static struct Node *primary(struct Token **rest, struct Token *tok);

static struct Node *new_node(enum NodeKind kind) {
    struct Node *node = calloc(1, sizeof(struct Node));
    node->kind = kind;
    return node;
};

static struct Node *new_binary(enum NodeKind kind, struct Node *lhs,
                               struct Node *rhs) {
    struct Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
};

static struct Node *new_num(int val) {
    struct Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
};

static struct Node *new_unary(enum NodeKind kind, struct Node *expr) {
    struct Node *node = new_node(kind);
    node->lhs = expr;
    return node;
};

// expr = equality
static struct Node *expr(struct Token **rest, struct Token *token) {
    return equality(rest, token);
};

// equality = relational ("==" relational | "!=" relational)*
static struct Node *equality(struct Token **rest, struct Token *token) {
    struct Node *node = relational(&token, token);

    for (;;) {
        if (equal(token, "==")) {
            node = new_binary(ND_EQ, node, relational(&token, token->next));
            continue;
        }

        if (equal(token, "!=")) {
            node = new_binary(ND_NE, node, relational(&token, token->next));
            continue;
        }
        *rest = token;
        return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static struct Node *relational(struct Token **rest, struct Token *token) {
    struct Node *node = add(&token, token);

    for (;;) {
        if (equal(token, "<")) {
            node = new_binary(ND_LT, node, add(&token, token->next));
            continue;
        }

        if (equal(token, "<=")) {
            node = new_binary(ND_LE, node, add(&token, token->next));
            continue;
        }

        if (equal(token, ">")) {
            node = new_binary(ND_LT, add(&token, token->next), node);
            continue;
        }

        if (equal(token, ">=")) {
            node = new_binary(ND_LE, add(&token, token->next), node);
            continue;
        }

        *rest = token;
        return node;
    }
}

// add = mul ("+" mul | "-" mul)*
static struct Node *add(struct Token **rest, struct Token *token) {
    struct Node *node = mul(&token, token);

    for (;;) {
        if (equal(token, "+")) {
            node = new_binary(ND_ADD, node, mul(&token, token->next));
            continue;
        }

        if (equal(token, "-")) {
            node = new_binary(ND_SUB, node, mul(&token, token->next));
            continue;
        }

        *rest = token;
        return node;
    }
};

// mul = unary ("*" unary | "/" unary)*
static struct Node *mul(struct Token **rest, struct Token *token) {
    struct Node *node = unary(&token, token);
    for (;;) {
        if (equal(token, "*")) {
            node = new_binary(ND_MUL, node, unary(&token, token->next));
            continue;
        }

        if (equal(token, "/")) {
            node = new_binary(ND_DIV, node, unary(&token, token->next));
            continue;
        }

        *rest = token;
        return node;
    }
};

// unary = ("+" | "-")? unary
//       | primary
static struct Node *unary(struct Token **rest, struct Token *token) {
    if (equal(token, "+")) {
        return unary(rest, token->next);
    }
    if (equal(token, "-")) {
        return new_unary(ND_NEG, unary(rest, token->next));
    }
    return primary(rest, token);
}

// primary = "(" expr ")" | num
static struct Node *primary(struct Token **rest, struct Token *token) {
    if (equal(token, "(")) {
        struct Node *node = expr(&token, token->next);
        *rest = skip(token, ")");
        return node;
    }

    if (token->kind == TK_NUM) {
        struct Node *node = new_num(token->val);
        *rest = token->next;
        return node;
    }

    error_tok(token, "Expected an expression");
};

struct Node *parse(struct Token *token) {
    struct Node *node = expr(&token, token);
    if (token->kind != TK_EOF) {
        error_tok(token, "Extra token");
    }
    return node;
}
