#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Pipe.h"

namespace Sequent
{
    class Process
    {
    private:
        int processId;
        std::shared_ptr<Pipe> standardIn;
        std::shared_ptr<Pipe> standardOut;
        std::shared_ptr<Pipe> standardError;
        bool hasExited;
        int exitStatus;

        Process(int processId, std::shared_ptr<Pipe> standardIn, std::shared_ptr<Pipe> standardOut, std::shared_ptr<Pipe> standardError);

    public:
        static std::unique_ptr<Process> Execute(std::string command, std::string argument1);
        static std::unique_ptr<Process> Execute(std::string command, std::string argument1, std::string argument2);
        static std::unique_ptr<Process> Execute(std::string command, std::string argument1, std::string argument2, std::string argument3);
        static std::unique_ptr<Process> Execute(std::string command, std::string argument1, std::string argument2, std::string argument3, std::string argument4);
        static std::unique_ptr<Process> Execute(std::string command, std::vector<std::string> &arguments);

        int GetProcessId();
        Pipe &GetStandardIn();
        Pipe &GetStandardOut();
        Pipe &GetStandardError();
        void Wait();
        int GetExitStatus();
    };
}

