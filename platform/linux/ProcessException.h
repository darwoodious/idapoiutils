#pragma once

#include <string>
#include <exception>

#include "BasicException.h"

namespace Sequent
{
    class ProcessException : public BasicException
    {
    public:
        ProcessException()
        : BasicException()
        { }

        ProcessException(const char *message)
        : BasicException(message)
        { }

        ProcessException(std::string message)
        : BasicException(message)
        { }

        const char *type() const noexcept override
        {
            return "ProcessException";
        }
    };
}
