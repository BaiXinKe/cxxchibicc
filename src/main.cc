#include "cxxchibicc.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments", argv[0]);
        return 1;
    }
    try {
        auto tok = tokenize(argv[1]);
        auto node = parse(std::move(tok));
        codegen(std::move(node));
    } catch (const std::exception& ex) {
        fprintf(stderr, "%s\n", ex.what());
    }
    return 0;
}