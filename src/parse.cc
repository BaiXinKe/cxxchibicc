#include "ChibiccException.h"
#include "cxxchibicc.h"
#include <memory>

static NodePtr expr(TokenPtr& rest, TokenPtr& tok);
static NodePtr expr_stmt(TokenPtr& rest, TokenPtr& tok);
static NodePtr equality(TokenPtr& rest, TokenPtr& tok);
static NodePtr relational(TokenPtr& rest, TokenPtr& tok);
static NodePtr add(TokenPtr& rest, TokenPtr& tok);
static NodePtr mul(TokenPtr& rest, TokenPtr& tok);
static NodePtr unary(TokenPtr& rest, TokenPtr& tok);
static NodePtr primary(TokenPtr& rest, TokenPtr& tok);

static NodePtr stmt(TokenPtr& rest, TokenPtr& tok)
{
    return expr_stmt(rest, tok);
}

static NodePtr expr_stmt(TokenPtr& rest, TokenPtr& tok)
{
    NodePtr node = std::make_unique<Node>(NodeKind::EXPR_STMT, expr(tok, tok), nullptr);
    rest = skip(tok, ";");
    return node;
}

NodePtr expr(TokenPtr& rest, TokenPtr& tok)
{
    return equality(rest, tok);
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

    if (equal(tok, "(")) {
        NodePtr node = expr(tok, tok->next);
        rest = skip(tok, ")");
        return node;
    }

    throw ChibiccException { tok->loc, "expected an expression" };
}

NodePtr parse(TokenPtr tok)
{
    Node node {};
    Node* cur = &node;

    while (tok->kind != TokenKind::END) {
        cur->next = stmt(tok, tok);
        cur = cur->next.get();
    }
    return std::move(node.next);
}