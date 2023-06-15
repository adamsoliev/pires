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

static void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

static bool equal(struct Token *token, char *op) {
    return memcmp(token->loc, op, token->len) == 0 && op[token->len] == '\0';
}

static struct Token *skip(struct Token *token, char *op) {
    if (!equal(token, op)) {
        error("Expected '%s'", op);
    }
    return token->next;
}

static int get_number(struct Token *token) {
    if (token->kind != TK_NUM) {
        error("Expected a number");
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

static struct Token *tokenize(char *p) {
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

        if (*p == '+' || *p == '-') {
            cur = cur->next = new_token(TK_PUNCT, p, p + 1);
            p++;
            continue;
        }

        error("Invalid token");
    }
    cur = cur->next = new_token(TK_EOF, p, p);
    return head.next;
}

int main(int argc, char *argv[]) {
    if (argc != 2) error("%s: invalid number of arguments\n", argv[0]);

    struct Token *token = tokenize(argv[1]);

    printf("    .globl main\n");
    printf("main:\n");

    printf("   li a0, %ld\n", get_number(token));
    token = token->next;

    while (token->kind != TK_EOF) {
        if (equal(token, "+")) {
            printf("   addi a0, a0, %ld\n", get_number(token->next));
            token = token->next->next;
            continue;
        }
        token = skip(token, "-");
        printf("   addi a0, a0, -%ld\n", get_number(token));
        token = token->next;
    }
    printf("   ret\n");
    return 0;
}