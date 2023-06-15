#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenize.c

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

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(struct Token *tok, char *fmt, ...);
bool equal(struct Token *tok, char *op);
struct Token *skip(struct Token *tok, char *op);
struct Token *tokenize(char *input);

// parse.c

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

struct Node *parse(struct Token *tok);

// codegen.c

void codegen(struct Node *node);
