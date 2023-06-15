#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum TokenKind {
    TK_PUNCT,
    TK_NUM,
    TK_EOF,
};

struct Token {
    enum TokenKind kind;
    struct Token *next;
    int val;
    char *loc;
    int len;
};

static char *current_input;

static void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

static void verror_at(char *loc, char *fmt, va_list ap) {
    int pos = loc - current_input;
    fprintf(stderr, "%s\n", current_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

static void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(loc, fmt, ap);
}

static void error_tok(struct Token *token, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(token->loc, fmt, ap);
}

static bool equal(struct Token *token, char *op) {
    return memcmp(token->loc, op, token->len) == 0 && op[token->len] == '\0';
}

static struct Token *skip(struct Token *token, char *s) {
    if (!equal(token, s)) {
        error_tok(token, "Expected '%s'", s);
    }
    return token->next;
}

static int get_number(struct Token *token) {
    if (token->kind != TK_NUM) {
        error_tok(token, "Expected a number");
    }
    return token->val;
}

static struct Token *new_token(enum TokenKind kind, char *start, char *end) {
    struct Token *token = calloc(1, sizeof(struct Token));
    token->kind = kind;
    token->loc = start;
    token->len = end - start;
    return token;
}

static bool startswith(char *p, char *q) {
    return strncmp(p, q, strlen(q)) == 0;
}

static int read_punct(char *p) {
    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
        startswith(p, ">=")) {
        return 2;
    }
    return ispunct(*p) ? 1 : 0;
}

static struct Token *tokenize(void) {
    char *p = current_input;
    struct Token head = {};
    struct Token *cur = &head;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (isdigit(*p)) {
            cur = cur->next = new_token(TK_NUM, p, p);
            char *q = p;
            cur->val = strtoul(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        int punct_len = read_punct(p);
        if (punct_len) {
            cur = cur->next = new_token(TK_PUNCT, p, p + punct_len);
            p += punct_len;
            continue;
        }

        error_at(p, "Invalid token");
    }
    cur = cur->next = new_token(TK_EOF, p, p);
    return head.next;
}

enum NodeKind {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NEG,  // unary -
    ND_EQ,   // ==
    ND_NE,   // !=
    ND_LT,   // <
    ND_LE,   // <=
    ND_NUM,
};

struct Node {
    enum NodeKind kind;
    struct Node *lhs;
    struct Node *rhs;
    int val;
};

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

static struct Node *expr(struct Token **rest, struct Token *tok);
static struct Node *equality(struct Token **rest, struct Token *tok);
static struct Node *relational(struct Token **rest, struct Token *tok);
static struct Node *add(struct Token **rest, struct Token *tok);
static struct Node *mul(struct Token **rest, struct Token *tok);
static struct Node *unary(struct Token **rest, struct Token *tok);
static struct Node *primary(struct Token **rest, struct Token *tok);

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

// codegen
static int depth;

static void push(void) {
    printf("  addi sp, sp, -8\n");
    printf("  sd a0, 0(sp)\n");
    depth++;
};

static void pop(char *reg) {
    printf("  ld %s, 0(sp)\n", reg);
    printf("  addi sp, sp, 8\n");
    depth--;
};

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

int main(int argc, char *argv[]) {
    if (argc != 2) error("%s: invalid number of arguments\n", argv[0]);

    current_input = argv[1];
    struct Token *token = tokenize();
    struct Node *node = expr(&token, token);

    if (token->kind != TK_EOF) error_tok(token, "Extra token");

    printf("    .globl main\n");
    printf("main:\n");

    gen_expr(node);
    printf("   ret\n");

    assert(depth == 0);
    return 0;
}