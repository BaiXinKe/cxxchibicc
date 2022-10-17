#include "cxxchibicc.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments", argv[0]);
        return 1;
    }

    TokenPtr tok = tokenize(argv[1]);
    NodePtr node = parse(std::move(tok));
    codegen(std::move(node));
    return 0;
}