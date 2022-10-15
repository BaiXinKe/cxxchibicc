#include <iostream>

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << argv[0] << ": invaild number of arguments";
        return 1;
    }

    std::cout << "  .global main\n";
    std::cout << "main:\n";

    std::cout << " mov $" << argv[1] << ", %rax\n";
    std::cout << " ret\n";

    return 0;
}