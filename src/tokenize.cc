#include "ChibiccException.h"
#include "cxxchibicc.h"

#include <cstring>
#include <iomanip>
#include <sstream>

static char* current_token { nullptr };

ChibiccException::ChibiccException(const char* loc, std::string_view message)
    : std::runtime_error { "" }
{
    std::ostringstream ofs;
    ofs << std::string { current_token, ::strlen(current_token) } << "\n"
        << std::setw(loc - current_token + 2) << "^ "
        << message;
    error_message_ = ofs.str();
}

// Consumes the current token if it matches `s`.
bool equal(const TokenPtr& tok, const char* op)
{
    return memcmp(tok->loc, op, tok->len) == 0
        && op[tok->len] == '\0';
}

// Ensure the current token if it matches `s`.
TokenPtr skip(const TokenPtr& tok, const char* s)
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

static bool startswidth(const char* p, const char* q)
{
    return strncmp(p, q, strlen(q)) == 0;
}

static bool is_ident1(char p)
{
    return (p >= 'a' && p <= 'z') || (p >= 'A' && p <= 'Z') || p == '_';
}

static bool is_ident2(char p)
{
    return is_ident1(p) || (p >= '0' && p <= '9');
}

static int read_punct(const char* p)
{
    if (startswidth(p, "==") || startswidth(p, "!=") || startswidth(p, "<=")
        || startswidth(p, ">="))
        return 2;
    return ispunct(*p) ? 1 : 0;
}

static void convert_keywords(TokenPtr& tok)
{
    auto inner_equal = [](Token* t) {
        const char* return_str = "return";
        return memcmp(t->loc, return_str, t->len) == 0
            && return_str[t->len] == '\0';
    };

    for (Token* ptok = tok.get(); ptok != nullptr; ptok = ptok->next.get()) {
        if (inner_equal(ptok)) {
            ptok->kind = TokenKind::KEYWORDS;
        }
    }
}

// Tokenize `current_input` and returns new tokens
TokenPtr tokenize(char* p)
{
    current_token = p;
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

        // Identity
        if (is_ident1(*p)) {
            char* start = p;
            do {
                p++;
            } while (is_ident2(*p));
            cur->next = std::make_unique<Token>(TokenKind::IDENT, start, p);
            cur = cur->next.get();
            continue;
        }

        // Punctuators
        int punct_len = read_punct(p);
        if (punct_len > 0) {
            cur->next = std::make_unique<Token>(TokenKind::PUNCT, p, p + punct_len);
            cur = cur->next.get();
            p += punct_len;
            continue;
        }

        throw ChibiccException { p, "invalid token" };
    }

    cur->next = std::make_unique<Token>(TokenKind::END, p, p);
    convert_keywords(head.next);
    return std::move(head.next);
}
