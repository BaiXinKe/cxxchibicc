#include "ChibiccException.h"
#include "cxxchibicc.h"
#include <memory>

static Obj locals;

static Obj* find_var(const TokenPtr& tok)
{
    Obj* cur = locals.next.get();
    while (cur != nullptr) {
        if (cur->name.size() == tok->len
            && cur->name == std::string(tok->loc, tok->len))
            return cur;
        cur = cur->next.get();
    }
    return nullptr;
}

static NodePtr compound_stmt(TokenPtr& rest, TokenPtr& tok);
static NodePtr expr(TokenPtr& rest, TokenPtr& tok);
static NodePtr expr_stmt(TokenPtr& rest, TokenPtr& tok);
static NodePtr assign(TokenPtr& rest, TokenPtr& tok);
static NodePtr equality(TokenPtr& rest, TokenPtr& tok);
static NodePtr relational(TokenPtr& rest, TokenPtr& tok);
static NodePtr add(TokenPtr& rest, TokenPtr& tok);
static NodePtr mul(TokenPtr& rest, TokenPtr& tok);
static NodePtr unary(TokenPtr& rest, TokenPtr& tok);
static NodePtr primary(TokenPtr& rest, TokenPtr& tok);

static NodePtr stmt(TokenPtr& rest, TokenPtr& tok)
{
    if (tok->kind == TokenKind::KEYWORDS) {
        NodePtr node = std::make_unique<Node>(NodeKind::RETURN, expr(tok, tok->next), nullptr);
        rest = skip(tok, ";");
        return node;
    }

    if (equal(tok, "{"))
        return compound_stmt(rest, tok->next);

    return expr_stmt(rest, tok);
}

static NodePtr compound_stmt(TokenPtr& rest, TokenPtr& tok)
{
    Node head = {};
    Node* cur = &head;
    while (!equal(tok, "}")) {
        cur->next = stmt(tok, tok);
        cur = cur->next.get();
    }

    NodePtr node = std::make_unique<Node>(NodeKind::BLOCK);
    node->body = std::move(head.next);
    rest = std::move(tok->next);
    return node;
}

static NodePtr expr_stmt(TokenPtr& rest, TokenPtr& tok)
{
    if (equal(tok, ";")) {
        rest = std::move(tok->next);
        return std::make_unique<Node>(NodeKind::BLOCK);
    }

    NodePtr node = std::make_unique<Node>(NodeKind::EXPR_STMT, expr(tok, tok), nullptr);
    rest = skip(tok, ";");
    return node;
}

NodePtr expr(TokenPtr& rest, TokenPtr& tok)
{
    return assign(rest, tok);
}

NodePtr assign(TokenPtr& rest, TokenPtr& tok)
{
    NodePtr node = equality(tok, tok);

    if (equal(tok, "=")) {
        node = std::make_unique<Node>(NodeKind::ASSIG, std::move(node), assign(tok, tok->next));
    }
    rest = std::move(tok);
    return node;
}

NodePtr equality(TokenPtr& rest, TokenPtr& tok)
{
    NodePtr node = relational(tok, tok);

    for (;;) {
        if (equal(tok, "==")) {
            node = std::make_unique<Node>(NodeKind::EQ, std::move(node), relational(tok, tok->next));
            continue;
        }

        if (equal(tok, "!=")) {
            node = std::make_unique<Node>(NodeKind::NE, std::move(node), relational(tok, tok->next));
            continue;
        }

        rest = std::move(tok);
        return node;
    }
}

NodePtr relational(TokenPtr& rest, TokenPtr& tok)
{
    NodePtr node = add(tok, tok);

    for (;;) {

        if (equal(tok, "<")) {
            node = std::make_unique<Node>(NodeKind::LT, std::move(node), add(tok, tok->next));
            continue;
        }

        if (equal(tok, "<=")) {
            node = std::make_unique<Node>(NodeKind::LE, std::move(node), add(tok, tok->next));
            continue;
        }

        if (equal(tok, ">")) {
            node = std::make_unique<Node>(NodeKind::LT, add(tok, tok->next), std::move(node));
            continue;
        }

        if (equal(tok, ">=")) {
            node = std::make_unique<Node>(NodeKind::LE, add(tok, tok->next), std::move(node));
            continue;
        }

        rest = std::move(tok);
        return node;
    }
}

NodePtr add(TokenPtr& rest, TokenPtr& tok)
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

        rest = std::move(tok);
        return node;
    }
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
        NodePtr node = std::make_unique<Node>(tok->value);
        rest = std::move(tok->next);
        return node;
    }

    if (tok->kind == TokenKind::IDENT) {
        Obj* var = find_var(tok);
        if (var == nullptr) {
            var = new Obj { tok->loc, tok->len };
            ObjPtr ptr { var };
            var->next = std::move(locals.next);
            locals.next = std::move(ptr);
        }
        rest = std::move(tok->next);
        return std::make_unique<Node>(var);
    }

    if (equal(tok, "(")) {
        NodePtr node = expr(tok, tok->next);
        rest = skip(tok, ")");
        return node;
    }

    throw ChibiccException { tok->loc, "expected an expression" };
}

FunctionPtr parse(TokenPtr tok)
{
    tok = skip(tok, "{");
    return std::make_unique<Function>(std::move(locals.next), compound_stmt(tok, tok));
}