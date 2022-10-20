#pragma once
#include <memory>

enum class TokenKind {
    IDENT,
    PUNCT,
    KEYWORDS,
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
    RETURN, // return
    VAR, // Variable
    NUM, // Integer
};

struct Obj {
    std::unique_ptr<Obj> next;
    std::string name;
    int offset;

    Obj() = default;

    Obj(const char* n, const int len)
        : next { nullptr }
        , name { std::string(n, len) }
        , offset {}
    {
    }
};
using ObjPtr = std::unique_ptr<Obj>;

struct Node;
using NodePtr = std::unique_ptr<Node>;

struct Node {
    NodeKind kind;
    NodePtr next;
    NodePtr left;
    NodePtr right;
    int value;
    Obj* obj;

    Node() = default;
    explicit Node(NodeKind kind)
        : kind { kind }
        , left { nullptr }
        , right { nullptr }
        , value { 0 }
        , obj { nullptr }
    {
    }

    Node(NodeKind kind, NodePtr left, NodePtr right)
        : kind { kind }
        , left { std::move(left) }
        , right { std::move(right) }
        , value { 0 }
        , obj { nullptr }
    {
    }

    Node(Obj* o)
        : kind { NodeKind::VAR }
        , next { nullptr }
        , left { nullptr }
        , right { nullptr }
        , value {}
        , obj { o }
    {
    }

    explicit Node(int value)
        : kind { NodeKind::NUM }
        , left { nullptr }
        , right { nullptr }
        , value(value)
        , obj { nullptr }
    {
    }

    explicit Node(char c)
        : kind { NodeKind::VAR }
        , left { nullptr }
        , right { nullptr }
        , value {}
        , obj { nullptr }
    {
    }
};

struct Function {
    std::unique_ptr<Obj> locals;
    std::unique_ptr<Node> body;
    int stack_size;

    Function(std::unique_ptr<Obj> local, std::unique_ptr<Node> body)
        : locals { std::move(local) }
        , body { std::move(body) }
        , stack_size { 0 }
    {
    }
};
using FunctionPtr = std::unique_ptr<Function>;

FunctionPtr parse(TokenPtr tok);

void codegen(FunctionPtr node);