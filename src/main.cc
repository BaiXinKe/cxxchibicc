#include <cassert>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>

static char* current_token { nullptr };

class ChibiccException : public std::runtime_error {
public:
    explicit ChibiccException(std::string_view message)
        : std::runtime_error { "" }
        , error_message_ { std::string(message.data(), message.length()) }
    {
    }

    ChibiccException(const char* loc, std::string_view message)
        : std::runtime_error { "" }
    {
        std::ostringstream ofs;
        ofs << std::string { current_token, ::strlen(current_token) } << "\n"
            << std::setw(loc - current_token + 2) << "^ "
            << message;
        error_message_ = ofs.str();
    }

    virtual const char* what() const noexcept override
    {
        return error_message_.c_str();
    }

private:
    std::string error_message_;
};

enum class TokenKind {
    PUNCT,
    NUM,
    END
};

struct Token;
using TokenPtr = std::unique_ptr<Token>;

struct Token {
    TokenKind kind; // Token kind
    TokenPtr next;
    const char* loc;
    int value;
    int len;

    Token() = default;
    Token(TokenKind kind, const char* start, const char* end)
        : kind { kind }
        , next { nullptr }
        , loc { start }
        , value { 0 }
        , len { static_cast<int>(end - start) }
    {
    }
};

// Consumes the current token if it matches `s`.
static bool equal(const TokenPtr& tok, const char* op)
{
    return memcmp(tok->loc, op, tok->len) == 0
        && op[tok->len] == '\0';
}

// Ensure the current token if it matches `s`.
static TokenPtr skip(const TokenPtr& tok, const char* s)
{
    if (!equal(tok, s)) {
        throw ChibiccException { tok->loc, "expected '" + std::string(s) + "'" };
    }
    return std::move(tok->next);
}

static int get_number(const TokenPtr& tok)
{
    if (tok->kind != TokenKind::NUM) {
        throw ChibiccException { tok->loc, "expected a number" };
    }
    return tok->value;
}

// Tokenize `current_input` and returns new tokens
static TokenPtr tokenize(void)
{
    char* p = current_token;
    Token head = {};
    Token* cur = &head;

    while (*p) {
        // Skip whitespace characters.
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Number literal
        if (isdigit(*p)) {
            cur->next = std::make_unique<Token>(TokenKind::NUM, p, p);
            cur = cur->next.get();
            char* q = p;
            cur->value = strtoul(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        // Punctuators
        if (ispunct(*p)) {
            cur->next = std::make_unique<Token>(TokenKind::PUNCT, p, p + 1);
            cur = cur->next.get();
            p++;
            continue;
        }

        throw ChibiccException { p, "invalid token" };
    }

    cur->next = std::make_unique<Token>(TokenKind::END, p, p);
    return std::move(head.next);
}

// --------------------------------------- AST Node --------------------------------------------
enum class NodeKind {
    ADD, // +
    SUB, // -
    MUL, // *
    DIV, // /
    NEG, // negtive integer
    NUM, // Integer
};

struct Node;
using NodePtr = std::unique_ptr<Node>;

struct Node {
    NodeKind kind;
    NodePtr left;
    NodePtr right;
    int value;

    explicit Node(NodeKind kind)
        : kind { kind }
        , left { nullptr }
        , right { nullptr }
        , value { 0 }
    {
    }

    Node(NodeKind kind, NodePtr left, NodePtr right)
        : kind { kind }
        , left { std::move(left) }
        , right { std::move(right) }
        , value { 0 }
    {
    }

    Node(int value)
        : kind { NodeKind::NUM }
        , left { nullptr }
        , right { nullptr }
        , value(value)
    {
    }
};

//
// Code generator
//
static int depth {};

static void push(void)
{
    printf("  push %%rax\n");
    depth++;
}

static void pop(const char* arg)
{
    printf("  pop %s\n", arg);
    depth--;
}

static void gen_expr(NodePtr node)
{

    switch (node->kind) {
    case NodeKind::NUM:
        printf("  mov $%d, %%rax\n", node->value);
        return;
    case NodeKind::NEG:
        gen_expr(std::move(node->left));
        printf(" neg %%rax\n");
        return;
    default:
        break;
    }

    gen_expr(std::move(node->right));
    push();
    gen_expr(std::move(node->left));
    pop("%rdi");

    switch (node->kind) {
    case NodeKind::ADD:
        printf("  add %%rdi, %%rax\n");
        return;
    case NodeKind::SUB:
        printf("  sub %%rdi, %%rax\n");
        return;
    case NodeKind::MUL:
        printf("  imul %%rdi, %%rax\n");
        return;
    case NodeKind::DIV:
        printf("  cqo\n");
        printf("  idiv %%rdi\n");
        return;
    default:
        break;
    };

    throw ChibiccException { "Invalid expression" };
}

static NodePtr expr(TokenPtr& rest, TokenPtr& tok);
static NodePtr mul(TokenPtr& rest, TokenPtr& tok);
static NodePtr unary(TokenPtr& rest, TokenPtr& tok);
static NodePtr primary(TokenPtr& rest, TokenPtr& tok);

NodePtr expr(TokenPtr& rest, TokenPtr& tok)
{
    NodePtr node = mul(tok, tok);

    for (;;) {
        if (equal(tok, "+")) {
            node = std::make_unique<Node>(NodeKind::ADD, std::move(node), mul(tok, tok->next));
            continue;
        }

        if (equal(tok, "-")) {
            node = std::make_unique<Node>(NodeKind::SUB, std::move(node), mul(tok, tok->next));
            continue;
        }
        break;
    }
    rest = std::move(tok);
    return node;
}

NodePtr mul(TokenPtr& rest, TokenPtr& tok)
{
    NodePtr node = unary(tok, tok);

    for (;;) {
        if (equal(tok, "*")) {
            node = std::make_unique<Node>(NodeKind::MUL, std::move(node), unary(tok, tok->next));
            continue;
        }

        if (equal(tok, "/")) {
            node = std::make_unique<Node>(NodeKind::DIV, std::move(node), unary(tok, tok->next));
            continue;
        }

        break;
    }
    rest = std::move(tok);
    return node;
}

NodePtr unary(TokenPtr& rest, TokenPtr& tok)
{
    if (equal(tok, "+")) {
        return unary(rest, tok->next);
    }

    if (equal(tok, "-")) {
        return std::make_unique<Node>(NodeKind::NEG, unary(rest, tok->next), nullptr);
    }

    return primary(rest, tok);
}

NodePtr primary(TokenPtr& rest, TokenPtr& tok)
{
    if (tok->kind == TokenKind::NUM) {
        NodePtr node = std::make_unique<Node>(get_number(tok));
        rest = std::move(tok->next);
        return node;
    }

    if (equal(tok, "(")) {
        NodePtr node = expr(tok, tok->next);
        rest = skip(tok, ")");
        return node;
    }

    throw ChibiccException { tok->loc, "expected an expression" };
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments", argv[0]);
        return 1;
    }

    current_token = argv[1];
    TokenPtr tok = tokenize();
    NodePtr node = expr(tok, tok);

    if (tok->kind != TokenKind::END)
        throw ChibiccException { tok->loc, "extra token" };

    printf("  .global main\n");
    printf("main:\n");

    gen_expr(std::move(node));
    printf("  ret\n");

    assert(depth == 0);
    return 0;
}