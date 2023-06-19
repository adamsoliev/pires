#include "pires.h"

static char *current_input;

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void verror_at(char *loc, char *fmt, va_list ap) {
    int pos = loc - current_input;
    fprintf(stderr, "%s\n", current_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(loc, fmt, ap);
}

void error_tok(struct Token *token, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(token->loc, fmt, ap);
}

bool equal(struct Token *token, char *op) {
    return memcmp(token->loc, op, token->len) == 0 && op[token->len] == '\0';
}

struct Token *skip(struct Token *token, char *s) {
    if (!equal(token, s)) {
        error_tok(token, "Expected '%s'", s);
    }
    return token->next;
}

bool consume(struct Token **rest, struct Token *token, char *s) {
    if (equal(token, s)) {
        *rest = token->next;
        return true;
    }
    *rest = token;
    return false;
}

int get_number(struct Token *token) {
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

static bool is_ident1(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_ident2(char c) { return is_ident1(c) || ('0' <= c && c <= '9'); }

static int read_punct(char *p) {
    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
        startswith(p, ">=")) {
        return 2;
    }
    return ispunct(*p) ? 1 : 0;
}

static bool is_keyword(struct Token *token) {
    static char *kw[] = {"return", "if", "else", "for", "while", "int"};

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
        if (equal(token, kw[i])) {
            return true;
        }
    }
    return false;
}

static void convert_keywords(struct Token *token) {
    for (struct Token *t = token; t->kind != TK_EOF; t = t->next) {
        if (is_keyword(t)) {
            t->kind = TK_KEYWORD;
        }
    }
}

struct Token *tokenize(char *p) {
    current_input = p;
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

        if (is_ident1(*p)) {
            char *start = p;
            do {
                p++;
            } while (is_ident2(*p));
            cur = cur->next = new_token(TK_IDENT, start, p);
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
    convert_keywords(head.next);
    return head.next;
}