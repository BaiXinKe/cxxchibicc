#include "ChibiccException.h"
#include "cxxchibicc.h"

#include <cassert>
#include <cstdio>
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

static void gen_addr(const NodePtr& node)
{
    if (node->kind == NodeKind::VAR) {
        int offset = (node->name - 'a' + 1) * 8;
        printf("  lea %d(%%rbp), %%rax\n", -offset);
        return;
    }
    throw ChibiccException { "not an lvalue" };
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
    case NodeKind::VAR:
        gen_addr(node);
        printf("  mov (%%rax), %%rax\n");
        return;
    case NodeKind::ASSIG:
        gen_addr(node->left);
        push();
        gen_expr(std::move(node->right));
        pop("%rdi");
        printf("  mov %%rax, (%%rdi)\n");
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
    case NodeKind::LT:
    case NodeKind::LE:
    case NodeKind::EQ:
    case NodeKind::NE: {
        printf("  cmp %%rdi, %%rax\n");

        if (node->kind == NodeKind::EQ) {
            printf("  sete %%al\n");
        } else if (node->kind == NodeKind::NE) {
            printf("  setne %%al\n");
        } else if (node->kind == NodeKind::LT) {
            printf("  setl %%al\n");
        } else if (node->kind == NodeKind::LE) {
            printf("  setle %%al\n");
        }

        printf("  movzb %%al, %%rax\n");
        return;
    }
    default:
        break;
    };

    throw ChibiccException { "Invalid expression" };
}

static void stmt_gen(NodePtr& node)
{
    if (node->kind == NodeKind::EXPR_STMT) {
        gen_expr(std::move(node->left));
        return;
    }
    throw ChibiccException { "invalid statement" };
}

void codegen(NodePtr node)
{
    printf(" .global main\n");
    printf("main:\n");

    printf("  push %%rbp\n");
    printf("  mov %%rsp, %%rbp\n");
    printf("  sub $208, %%rsp\n");

    for (NodePtr n = std::move(node); n != nullptr; n = std::move(n->next)) {
        stmt_gen(n);
        assert(depth == 0);
    }

    printf("  mov %%rbp, %%rsp\n");
    printf("  pop %%rbp\n");
    printf("  ret\n");
}