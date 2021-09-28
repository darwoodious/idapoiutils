#pragma once

#include <string>
#include <exception>

#include "BasicException.h"

namespace Sequent
{
    class ArgumentOutOfRangeException : public BasicException
    {
    public:
        ArgumentOutOfRangeException()
        : BasicException()
        { }

        ArgumentOutOfRangeException(const char *argumentName)
        : BasicException(argumentName)
        { }

        ArgumentOutOfRangeException(std::string argumentName)
        : BasicException(argumentName)
        { }

        const char *type() const noexcept override
        {
            return "ArgumentOutOfRangeException";
        }
    };
}
