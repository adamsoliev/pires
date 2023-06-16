
#include "luan.h"

static struct Node *compound_stmt(struct Token **rest, struct Token *tok);
static struct Node *expr(struct Token **rest, struct Token *tok);
static struct Node *expr_stmt(struct Token **rest, struct Token *tok);
static struct Node *assign(struct Token **rest, struct Token *tok);
static struct Node *equality(struct Token **rest, struct Token *tok);
static struct Node *relational(struct Token **rest, struct Token *tok);
static struct Node *add(struct Token **rest, struct Token *tok);
static struct Node *mul(struct Token **rest, struct Token *tok);
static struct Node *unary(struct Token **rest, struct Token *tok);
static struct Node *primary(struct Token **rest, struct Token *tok);

struct Obj *locals;

static struct Obj *find_var(struct Token *token) {
    for (struct Obj *var = locals; var; var = var->next) {
        if (strlen(var->name) == token->len &&
            !strncmp(token->loc, var->name, token->len)) {
            return var;
        }
    }
    return NULL;
}

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

static struct Node *new_var_node(struct Obj *var) {
    struct Node *node = new_node(ND_VAR);
    node->var = var;
    return node;
};

static struct Obj *new_lvar(char *name) {
    struct Obj *var = calloc(1, sizeof(struct Obj));
    var->name = name;
    var->next = locals;
    locals = var;
    return var;
}

static struct Node *new_unary(enum NodeKind kind, struct Node *expr) {
    struct Node *node = new_node(kind);
    node->lhs = expr;
    return node;
};

// stmt = "return" expr ";"
//      | "{" compound-stmt* "}"
//      | expr-stmt
static struct Node *stmt(struct Token **rest, struct Token *token) {
    if (equal(token, "return")) {
        struct Node *node = new_unary(ND_RETURN, expr(&token, token->next));
        *rest = skip(token, ";");
        return node;
    }
    if (equal(token, "if")) {
        struct Node *node = new_node(ND_IF);
        token = skip(token->next, "(");
        node->cond = expr(&token, token);
        token = skip(token, ")");
        node->then = stmt(&token, token);
        if (equal(token, "else")) {
            node->els = stmt(&token, token->next);
        }
        *rest = token;
        return node;
    }
    if (equal(token, "{")) {
        return compound_stmt(rest, token->next);
    }
    return expr_stmt(rest, token);
};

// compound-stmt =  stmt* "}"
static struct Node *compound_stmt(struct Token **rest, struct Token *token) {
    struct Node head = {};
    struct Node *cur = &head;

    while (!equal(token, "}")) {
        cur = cur->next = stmt(&token, token);
    }
    struct Node *node = new_node(ND_BLOCK);
    node->body = head.next;
    *rest = token->next;
    return node;
}

// expr-stmt = expr? ";"
static struct Node *expr_stmt(struct Token **rest, struct Token *token) {
    if (equal(token, ";")) {
        *rest = token->next;
        return new_node(ND_BLOCK);
    }

    struct Node *node = new_unary(ND_EXPR_STMT, expr(&token, token));
    *rest = skip(token, ";");
    return node;
};

// expr = assign
static struct Node *expr(struct Token **rest, struct Token *token) {
    return assign(rest, token);
};

// assign = equality ("=" assign)?
static struct Node *assign(struct Token **rest, struct Token *token) {
    struct Node *node = equality(&token, token);
    if (equal(token, "=")) {
        node = new_binary(ND_ASSIGN, node, assign(&token, token->next));
    }
    *rest = token;
    return node;
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

// primary = "(" expr ")" | ident | num
static struct Node *primary(struct Token **rest, struct Token *token) {
    if (equal(token, "(")) {
        struct Node *node = expr(&token, token->next);
        *rest = skip(token, ")");
        return node;
    }

    if (token->kind == TK_IDENT) {
        struct Obj *var = find_var(token);
        if (!var) {
            var = new_lvar(strndup(token->loc, token->len));
        }
        *rest = token->next;
        return new_var_node(var);
    }

    if (token->kind == TK_NUM) {
        struct Node *node = new_num(token->val);
        *rest = token->next;
        return node;
    }

    error_tok(token, "Expected an expression");
    return NULL;
};

// program = stmt*
struct Function *parse(struct Token *token) {
    token = skip(token, "{");

    struct Function *prog = calloc(1, sizeof(struct Function));
    prog->body = compound_stmt(&token, token);
    prog->locals = locals;
    return prog;
}
