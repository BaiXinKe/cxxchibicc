#pragma once

#include <stdexcept>
#include <string_view>

class ChibiccException : public std::runtime_error {
public:
    explicit ChibiccException(std::string_view message)
        : std::runtime_error { "" }
        , error_message_ { std::string(message.data(), message.length()) }
    {
    }

    ChibiccException(const char* loc, std::string_view message);

    virtual const char* what() const noexcept override
    {
        return error_message_.c_str();
    }

private:
    std::string error_message_;
};