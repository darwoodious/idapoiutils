#pragma once

#include <string>
#include <exception>

#include "BasicException.h"

namespace Sequent
{
    class IoException : public BasicException
    {
    public:
        IoException()
        : BasicException()
        { }

        IoException(const char *message)
        : BasicException(message)
        { }

        IoException(std::string message)
        : BasicException(message)
        { }

        const char *type() const noexcept override
        {
            return "IoException";
        }
    };
}
