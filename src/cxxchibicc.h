#pragma once
#include <memory>

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

bool equal(const TokenPtr& tok, const char* op);
TokenPtr skip(const TokenPtr& tok, const char* s);
TokenPtr tokenize(char* input);

// --------------------------------------- AST Node --------------------------------------------
enum class NodeKind {
    ADD, // +
    SUB, // -
    MUL, // *
    DIV, // /
    NEG, // negtive integer
    EQ, // ==
    NE, // !=
    LT, // <
    LE, // <=
    EXPR_STMT,
    NUM, // Integer
};

struct Node;
using NodePtr = std::unique_ptr<Node>;

struct Node {
    NodeKind kind;
    NodePtr next;
    NodePtr left;
    NodePtr right;
    int value;

    Node() = default;
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

NodePtr parse(TokenPtr tok);

void codegen(NodePtr node);