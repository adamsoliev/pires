#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenize.c

enum TokenKind {
    TK_IDENT,
    TK_PUNCT,
    TK_KEYWORD,
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

struct Obj {
    struct Obj *next;
    char *name;
    int offset;
};

struct Function {
    struct Node *body;
    struct Obj *locals;
    int stack_size;
};

enum NodeKind {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NEG,        // unary -
    ND_EQ,         // ==
    ND_NE,         // !=
    ND_LT,         // <
    ND_LE,         // <=
    ND_ASSIGN,     // =
    ND_RETURN,     // "return"
    ND_IF,         // "if"
    ND_BLOCK,      // { ... }
    ND_EXPR_STMT,  // Expression statement
    ND_VAR,        // Variable
    ND_NUM,
};

struct Node {
    enum NodeKind kind;
    struct Node *next;
    struct Node *lhs;
    struct Node *rhs;
    struct Obj *var;    // Used if kind == ND_VAR
    int val;            // Used if kind == ND_NUM
    struct Node *body;  // ND_BLOCK

    // "if" cond
    struct Node *cond;
    struct Node *then;
    struct Node *els;
};

struct Function *parse(struct Token *tok);

// codegen.c

void codegen(struct Function *node);
