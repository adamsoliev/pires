#include "luan.h"

int main(int argc, char *argv[]) {
    if (argc != 2) error("%s: invalid number of arguments\n", argv[0]);

    struct Token *token = tokenize(argv[1]);
    struct Node *node = parse(token);
    codegen(node);

    return 0;
}