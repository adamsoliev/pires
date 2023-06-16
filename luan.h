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
bool consume(struct Token **rest, struct Token *tok, char *str);
struct Token *tokenize(char *input);

// parse.c

struct Obj {
    struct Obj *next;
    char *name;
    struct Type *ty;
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
    ND_ADDR,       // unary &
    ND_DEREF,      // unary *
    ND_RETURN,     // "return"
    ND_IF,         // "if"
    ND_FOR,        // "for" or "while"
    ND_BLOCK,      // { ... }
    ND_FUNCALL,    // Function call
    ND_EXPR_STMT,  // Expression statement
    ND_VAR,        // Variable
    ND_NUM,
};

struct Node {
    enum NodeKind kind;
    struct Node *next;
    struct Type *ty;
    struct Token *token;
    struct Node *lhs;
    struct Node *rhs;
    struct Obj *var;    // Used if kind == ND_VAR
    int val;            // Used if kind == ND_NUM
    struct Node *body;  // ND_BLOCK

    char *funcname;     // Function call

    // "if" or "for" statement
    struct Node *cond;
    struct Node *then;
    struct Node *els;
    struct Node *init;
    struct Node *inc;
};

struct Function *parse(struct Token *tok);

// type.c

enum TypeKind {
    TY_INT,
    TY_PTR,
};

struct Type {
    enum TypeKind kind;
    struct Type *base;
    struct Token *name;
};

extern struct Type *ty_int;
bool is_integer(struct Type *ty);
struct Type *pointer_to(struct Type *base);
void add_type(struct Node *node);

// codegen.c

void codegen(struct Function *node);
