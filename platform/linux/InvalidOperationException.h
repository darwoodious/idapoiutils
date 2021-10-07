#pragma once

#include <string>
#include <exception>

#include "BasicException.h"

namespace Sequent
{
    class InvalidOperationException : public BasicException
    {
    public:
        InvalidOperationException()
        : BasicException()
        { }

        InvalidOperationException(const char *message)
        : BasicException(message)
        { }

        InvalidOperationException(std::string message)
        : BasicException(message)
        { }

        const char *type() const noexcept override
        {
            return "InvalidOperationException";
        }
    };
}