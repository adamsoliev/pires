
#include "luan.h"

static struct Type *declspec(struct Token **rest, struct Token *tok);
static struct Type *declarator(struct Token **rest, struct Token *tok,
                               struct Type *ty);
static struct Node *declaration(struct Token **rest, struct Token *tok);
static struct Node *compound_stmt(struct Token **rest, struct Token *tok);
static struct Node *stmt(struct Token **rest, struct Token *tok);
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

static struct Node *new_node(enum NodeKind kind, struct Token *token) {
    struct Node *node = calloc(1, sizeof(struct Node));
    node->kind = kind;
    node->token = token;
    return node;
};

static struct Node *new_binary(enum NodeKind kind, struct Node *lhs,
                               struct Node *rhs, struct Token *token) {
    struct Node *node = new_node(kind, token);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
};

static struct Node *new_num(int val, struct Token *token) {
    struct Node *node = new_node(ND_NUM, token);
    node->val = val;
    return node;
};

static struct Node *new_var_node(struct Obj *var, struct Token *token) {
    struct Node *node = new_node(ND_VAR, token);
    node->var = var;
    return node;
};

static struct Obj *new_lvar(char *name, struct Type *ty) {
    struct Obj *var = calloc(1, sizeof(struct Obj));
    var->name = name;
    var->ty = ty;
    var->next = locals;
    locals = var;
    return var;
}

static struct Node *new_unary(enum NodeKind kind, struct Node *expr,
                              struct Token *token) {
    struct Node *node = new_node(kind, token);
    node->lhs = expr;
    return node;
};

static char *get_ident(struct Token *token) {
    if (token->kind != TK_IDENT) {
        error_tok(token, "Expected an identifier");
    }
    return strndup(token->loc, token->len);
}

// declspec = "int"
static struct Type *declspec(struct Token **rest, struct Token *token) {
    *rest = skip(token, "int");
    return ty_int;
}

// type-suffix  = ("(" func-params)? ")")?
// func-params  = param ("," param)*
// param        = declspec declarator
static struct Type *type_suffix(struct Token **rest, struct Token *token,
                                struct Type *ty) {
    if (equal(token, "(")) {
        token = token->next;

        struct Type head = {};
        struct Type *cur = &head;

        while (!equal(token, ")")) {
            if (cur != &head) {
                token = skip(token, ",");
            }
            struct Type *basety = declspec(&token, token);
            struct Type *ty = declarator(&token, token, basety);
            cur = cur->next = copy_type(ty);
        }
        ty = func_type(ty);
        ty->params = head.next;
        *rest = token->next;
        return ty;
    }
    *rest = token;
    return ty;
}

// declarator = "*"* ident type-suffix
static struct Type *declarator(struct Token **rest, struct Token *token,
                               struct Type *ty) {
    while (consume(&token, token, "*")) {
        ty = pointer_to(ty);
    }
    if (token->kind != TK_IDENT) {
        error_tok(token, "Expected a variable name");
    }
    ty = type_suffix(rest, token->next, ty);
    ty->name = token;
    return ty;
}

// declaratoin = declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
static struct Node *declaration(struct Token **rest, struct Token *tok) {
    struct Type *basety = declspec(&tok, tok);

    struct Node head = {};
    struct Node *cur = &head;
    int i = 0;

    while (!equal(tok, ";")) {
        if (i++ > 0) tok = skip(tok, ",");

        struct Type *ty = declarator(&tok, tok, basety);
        struct Obj *var = new_lvar(get_ident(ty->name), ty);

        if (!equal(tok, "=")) continue;

        struct Node *lhs = new_var_node(var, ty->name);
        struct Node *rhs = assign(&tok, tok->next);
        struct Node *node = new_binary(ND_ASSIGN, lhs, rhs, tok);
        cur = cur->next = new_unary(ND_EXPR_STMT, node, tok);
    }

    struct Node *node = new_node(ND_BLOCK, tok);
    node->body = head.next;
    *rest = tok->next;
    return node;
}

// stmt = "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr-stmt expr? ";" expr? ")" stmt
//      | "while" "(" expr ")" stmt
//      | "{" compound-stmt
//      | expr-stmt
static struct Node *stmt(struct Token **rest, struct Token *token) {
    if (equal(token, "return")) {
        struct Node *node = new_node(ND_RETURN, token);
        node->lhs = expr(&token, token->next);
        *rest = skip(token, ";");
        return node;
    }
    if (equal(token, "if")) {
        struct Node *node = new_node(ND_IF, token);
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
    if (equal(token, "for")) {
        struct Node *node = new_node(ND_FOR, token);
        token = skip(token->next, "(");

        node->init = expr_stmt(&token, token);

        if (!equal(token, ";")) {
            node->cond = expr(&token, token);
        }
        token = skip(token, ";");

        if (!equal(token, ")")) {
            node->inc = expr(&token, token);
        }
        token = skip(token, ")");

        node->then = stmt(rest, token);
        return node;
    }
    if (equal(token, "while")) {
        struct Node *node = new_node(ND_FOR, token);
        token = skip(token->next, "(");
        node->cond = expr(&token, token);
        token = skip(token, ")");
        node->then = stmt(rest, token);
        return node;
    }
    if (equal(token, "{")) {
        return compound_stmt(rest, token->next);
    }
    return expr_stmt(rest, token);
};

// compound-stmt = (declaration | stmt)* "}"
static struct Node *compound_stmt(struct Token **rest, struct Token *token) {
    struct Node *node = new_node(ND_BLOCK, token);

    struct Node head = {};
    struct Node *cur = &head;

    while (!equal(token, "}")) {
        if (equal(token, "int")) {
            cur = cur->next = declaration(&token, token);
        } else {
            cur = cur->next = stmt(&token, token);
        }
        add_type(cur);
    }
    node->body = head.next;
    *rest = token->next;
    return node;
}

// expr-stmt = expr? ";"
static struct Node *expr_stmt(struct Token **rest, struct Token *token) {
    if (equal(token, ";")) {
        *rest = token->next;
        return new_node(ND_BLOCK, token);
    }

    struct Node *node = new_node(ND_EXPR_STMT, token);
    node->lhs = expr(&token, token);
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
        return new_binary(ND_ASSIGN, node, assign(rest, token->next), token);
    }
    *rest = token;
    return node;
};

// equality = relational ("==" relational | "!=" relational)*
static struct Node *equality(struct Token **rest, struct Token *token) {
    struct Node *node = relational(&token, token);

    for (;;) {
        struct Token *start = token;

        if (equal(token, "==")) {
            node =
                new_binary(ND_EQ, node, relational(&token, token->next), start);
            continue;
        }

        if (equal(token, "!=")) {
            node =
                new_binary(ND_NE, node, relational(&token, token->next), start);
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
        struct Token *start = token;
        if (equal(token, "<")) {
            node = new_binary(ND_LT, node, add(&token, token->next), start);
            continue;
        }

        if (equal(token, "<=")) {
            node = new_binary(ND_LE, node, add(&token, token->next), start);
            continue;
        }

        if (equal(token, ">")) {
            node = new_binary(ND_LT, add(&token, token->next), node, start);
            continue;
        }

        if (equal(token, ">=")) {
            node = new_binary(ND_LE, add(&token, token->next), node, start);
            continue;
        }

        *rest = token;
        return node;
    }
}

// take care of this "ptr1 + n => sizeof(*ptr1) * n" scaling
static struct Node *new_add(struct Node *lhs, struct Node *rhs,
                            struct Token *token) {
    add_type(lhs);
    add_type(rhs);

    // num + num
    if (is_integer(lhs->ty) && is_integer(rhs->ty)) {
        return new_binary(ND_ADD, lhs, rhs, token);
    }

    if (lhs->ty->base && rhs->ty->base) {
        error_tok(token, "Invalid operands");
    }

    // 'num + ptr' => 'ptr + num'
    if (!lhs->ty->base && rhs->ty->base) {
        struct Node *tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }

    // ptr + num
    rhs = new_binary(ND_MUL, rhs, new_num(8, token), token);
    return new_binary(ND_ADD, lhs, rhs, token);
}

// similar to new_add
static struct Node *new_sub(struct Node *lhs, struct Node *rhs,
                            struct Token *token) {
    add_type(lhs);
    add_type(rhs);

    // num - num
    if (is_integer(lhs->ty) && is_integer(rhs->ty)) {
        return new_binary(ND_SUB, lhs, rhs, token);
    }

    // ptr - num
    if (lhs->ty->base && is_integer(rhs->ty)) {
        rhs = new_binary(ND_MUL, rhs, new_num(8, token), token);
        add_type(rhs);
        struct Node *node = new_binary(ND_SUB, lhs, rhs, token);
        node->ty = lhs->ty;
        return node;
    }

    // ptr - ptr, which returns how many elements are between the two.
    if (lhs->ty->base && rhs->ty->base) {
        struct Node *node = new_binary(ND_SUB, lhs, rhs, token);
        node->ty = ty_int;
        return new_binary(ND_DIV, node, new_num(8, token), token);
    }

    error_tok(token, "Invalid operands");
    return NULL;
}

// add = mul ("+" mul | "-" mul)*
static struct Node *add(struct Token **rest, struct Token *token) {
    struct Node *node = mul(&token, token);

    for (;;) {
        struct Token *start = token;
        if (equal(token, "+")) {
            node = new_add(node, mul(&token, token->next), start);
            continue;
        }

        if (equal(token, "-")) {
            node = new_sub(node, mul(&token, token->next), start);
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
        struct Token *start = token;
        if (equal(token, "*")) {
            node = new_binary(ND_MUL, node, unary(&token, token->next), start);
            continue;
        }

        if (equal(token, "/")) {
            node = new_binary(ND_DIV, node, unary(&token, token->next), start);
            continue;
        }

        *rest = token;
        return node;
    }
};

// unary = ("+" | "-" | "*" | "&") unary
//       | primary
static struct Node *unary(struct Token **rest, struct Token *token) {
    if (equal(token, "+")) {
        return unary(rest, token->next);
    }
    if (equal(token, "-")) {
        return new_unary(ND_NEG, unary(rest, token->next), token);
    }
    if (equal(token, "&")) {
        return new_unary(ND_ADDR, unary(rest, token->next), token);
    }
    if (equal(token, "*")) {
        return new_unary(ND_DEREF, unary(rest, token->next), token);
    }
    return primary(rest, token);
}

// funccall = ident "(" (assign ("," assign)*)? ")"
static struct Node *funcall(struct Token **rest, struct Token *token) {
    struct Token *start = token;
    token = token->next->next;

    struct Node head = {};
    struct Node *cur = &head;

    while (!equal(token, ")")) {
        if (cur != &head) {
            token = skip(token, ",");
        }
        cur = cur->next = assign(&token, token);
    }

    *rest = skip(token, ")");

    struct Node *node = new_node(ND_FUNCALL, start);
    node->funcname = strndup(start->loc, start->len);
    node->args = head.next;
    return node;
}

// primary = "(" expr ")" | ident func-args? | num
static struct Node *primary(struct Token **rest, struct Token *token) {
    if (equal(token, "(")) {
        struct Node *node = expr(&token, token->next);
        *rest = skip(token, ")");
        return node;
    }

    if (token->kind == TK_IDENT) {
        // Function call
        if (equal(token->next, "(")) {
            return funcall(rest, token);
        }

        // Variable
        struct Obj *var = find_var(token);
        if (!var) {
            error_tok(token, "Undefined variable");
        }
        *rest = token->next;
        return new_var_node(var, token);
    }

    if (token->kind == TK_NUM) {
        struct Node *node = new_num(token->val, token);
        *rest = token->next;
        return node;
    }

    error_tok(token, "Expected an expression");
    return NULL;
};

static void create_param_lvars(struct Type *param) {
    if (param) {
        create_param_lvars(param->next);
        new_lvar(get_ident(param->name), param);
    }
}

static struct Function *function(struct Token **rest, struct Token *token) {
    struct Type *ty = declspec(&token, token);
    ty = declarator(&token, token, ty);

    locals = NULL;

    struct Function *fn = calloc(1, sizeof(struct Function));
    fn->name = get_ident(ty->name);
    create_param_lvars(ty->params);
    fn->params = locals;

    token = skip(token, "{");
    fn->body = compound_stmt(rest, token);
    fn->locals = locals;
    return fn;
}

// program = function-definition*
struct Function *parse(struct Token *token) {
    struct Function head = {};
    struct Function *cur = &head;
    while (token->kind != TK_EOF) {
        cur = cur->next = function(&token, token);
    }
    return head.next;
};
