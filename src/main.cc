#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string_view>

// Input string
static char* current_input;

enum class TokenKind : uint8_t {
    RESERVED,
    NUM,
    KEOF,
};

// Token type
struct Token {
    TokenKind kind; // Token kind
    std::unique_ptr<Token> next; // Next token
    int val; // If kind if num, its value
    char* loc; // Token location
    int len; // Token length

    Token() = default;
    Token(TokenKind k, char* str, int len)
        : kind { k }
        , next { nullptr }
        , loc { str }
        , len { len }
    {
    }
};

class CXXChibiccError : public std::runtime_error {
public:
    explicit CXXChibiccError(std::string_view msg)
        : std::runtime_error { "" }
        , message { msg }
    {
    }

    CXXChibiccError(char* loc, std::string_view msg)
        : runtime_error { "" }
    {
        std::ostringstream ifs;
        ifs << "\n"
            << current_input << "\n"
            << std::setw(loc - current_input + 2) << "^ "
            << msg;
        message = ifs.str();
    }
    CXXChibiccError(const std::unique_ptr<Token>& token, std::string_view msg)
        : CXXChibiccError { token->loc, msg }
    {
    }

    virtual const char* what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message {};
};

// Consumes the current token if it matches `s`.
static bool equal(const std::unique_ptr<Token>& tok, const char* op)
{
    return strlen(op) == tok->len
        && !strncmp(tok->loc, op, tok->len);
}

// Ensure that the current token is `s`
static std::unique_ptr<Token>& skip(const std::unique_ptr<Token>& tok, const char* s)
{
    if (!equal(tok, s)) {
        throw CXXChibiccError { tok, "expected '" + std::string(s) + "'" };
    }
    return tok->next;
}

// Ensure that the current token is NUM
static int get_number(const std::unique_ptr<Token>& tok)
{
    if (tok->kind != TokenKind::NUM) {
        throw CXXChibiccError { tok, "expected a number" };
    }
    return tok->val;
}

static std::unique_ptr<Token> tokenize(void)
{
    char* p = current_input;
    Token head {};
    Token* cur = &head;

    while (*p) {
        // Skip whitespace characters.
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Numeric literal
        if (isdigit(*p)) {
            cur->next = std::make_unique<Token>(TokenKind::NUM, p, 0);
            cur = cur->next.get();
            char* q = p;
            cur->val = strtoul(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        // Punctuator
        if (*p == '+' || *p == '-') {
            cur->next = std::make_unique<Token>(TokenKind::RESERVED, p++, 1);
            cur = cur->next.get();
            continue;
        }
    }

    cur->next = std::make_unique<Token>(TokenKind::KEOF, p, 0);
    return std::move(head.next);
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << argv[0] << ": invaild number of arguments";
        return 1;
    }

    current_input = argv[1];
    try {
        std::unique_ptr<Token> tok = tokenize();

        std::cout << "  .global main\n";
        std::cout << "main:\n";

        // The first token must be a number
        std::cout << " mov $" << get_number(tok) << ", %rax\n";
        tok = std::move(tok->next);

        // ... followed by either `+ <number>` or `- <number>`
        while (tok->kind != TokenKind::KEOF) {
            if (equal(tok, "+")) {
                std::cout << "  add $" << get_number(tok->next) << ", %rax\n";
                tok = std::move(tok->next->next);
                continue;
            }

            tok = std::move(skip(tok, "-"));
            std::cout << "  sub $" << get_number(tok) << ", %rax\n";
            tok = std::move(tok->next);
        }
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << "\n";
    }

    std::cout << "  ret\n";
    return 0;
}