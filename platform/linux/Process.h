#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Types.h"
#include "Pipe.h"

namespace Sequent
{
    enum struct Redirect : int
    {
        None = 0,
        StandardIn = 1,
        StandardOut = 2,
        StandardError = 4,
        Outputs = StandardOut | StandardError,
        All = StandardIn | StandardOut | StandardError
    };

    constexpr Redirect operator | (Redirect X, Redirect Y) {
        return static_cast<Redirect>(static_cast<int>(X) | static_cast<int>(Y));
    }

    constexpr Redirect operator & (Redirect X, Redirect Y) {
        return static_cast<Redirect>(static_cast<int>(X) & static_cast<int>(Y));
    }

    class Process
    {
    private:
        int processId;
        Redirect redirect;
        std::shared_ptr<Pipe> standardIn;
        std::shared_ptr<Pipe> standardOut;
        std::shared_ptr<Pipe> standardError;
        bool hasExited;
        int exitStatus;

        Process(int processId, Redirect rredirect, std::shared_ptr<Pipe> standardIn, std::shared_ptr<Pipe> standardOut, std::shared_ptr<Pipe> standardError);

    public:
        static std::unique_ptr<Process> Execute(std::string command, Redirect redirect, std::string argument1);
        static std::unique_ptr<Process> Execute(std::string command, Redirect redirect, std::string argument1, std::string argument2);
        static std::unique_ptr<Process> Execute(std::string command, Redirect redirect, std::string argument1, std::string argument2, std::string argument3);
        static std::unique_ptr<Process> Execute(std::string command, Redirect redirect, std::string argument1, std::string argument2, std::string argument3, std::string argument4);
        static std::unique_ptr<Process> Execute(std::string command, Redirect redirect, std::vector<std::string> &arguments);

        int GetProcessId();
        Redirect GetRedirect();
        Pipe &GetStandardIn();
        Pipe &GetStandardOut();
        Pipe &GetStandardError();
        void ExchangeStandardIo(uint8_vector *standardIn, uint8_vector *standardOut, uint8_vector *standardError);
        void Wait();
        int GetExitStatus();
    };
}
