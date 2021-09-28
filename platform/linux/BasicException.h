#pragma once

#include <exception>
#include <string>

namespace Sequent
{
    class BasicException : public std::exception
    {
    private:
        std::string message;

    public:
        BasicException()
        : exception()
        { }

        BasicException(const char *message)
        : exception()
        {
            this->message = message;
        }

        BasicException(std::string message)
        : exception()
        {
            this->message = message;
        }

        virtual const char *type() const noexcept
        {
            return "BasicException";
        }

        const char *what() const noexcept override
        {
            return message.c_str();
        }
    };
}