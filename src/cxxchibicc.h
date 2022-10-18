#pragma once
#include <memory>

enum class TokenKind {
    IDENT,
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
    ASSIG, // =
    EXPR_STMT,
    VAR, // Variable
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
    char name;

    Node() = default;
    explicit Node(NodeKind kind)
        : kind { kind }
        , left { nullptr }
        , right { nullptr }
        , value { 0 }
        , name {}
    {
    }

    Node(NodeKind kind, NodePtr left, NodePtr right)
        : kind { kind }
        , left { std::move(left) }
        , right { std::move(right) }
        , value { 0 }
        , name {}
    {
    }

    explicit Node(int value)
        : kind { NodeKind::NUM }
        , left { nullptr }
        , right { nullptr }
        , value(value)
        , name {}
    {
    }

    explicit Node(char c)
        : kind { NodeKind::VAR }
        , left { nullptr }
        , right { nullptr }
        , value {}
        , name { c }
    {
    }
};

NodePtr parse(TokenPtr tok);

void codegen(NodePtr node);