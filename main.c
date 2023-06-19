#include "pires.h"

int main(int argc, char *argv[]) {
    if (argc != 2) error("%s: invalid number of arguments\n", argv[0]);

    struct Token *token = tokenize(argv[1]);
    struct Function *prog = parse(token);
    codegen(prog);

    return 0;
}