#pragma once

#include <string>
#include <exception>

#include "BasicException.h"

namespace Sequent
{
    class EndOfStreamException : public BasicException
    {
    public:
        EndOfStreamException()
        : BasicException()
        { }

        EndOfStreamException(const char *message)
        : BasicException(message)
        { }

        EndOfStreamException(std::string message)
        : BasicException(message)
        { }

        const char *type() const noexcept override
        {
            return "EndOfStreamException";
        }
    };
}
