#include <memory>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <stdarg.h>

#include "Pipe.h"
#include "IoException.h"
#include "ProcessException.h"
#include "InvalidOperationException.h"

#include "Process.h"

using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::make_unique;
using std::vector;
using std::tuple;

namespace Sequent
{
    Process::Process(int processId, shared_ptr<Pipe> standardIn, shared_ptr<Pipe> standardOut, shared_ptr<Pipe> standardError)
    : processId(processId), standardIn(standardIn), standardOut(standardOut), standardError(standardError), hasExited(false)
    { }

    unique_ptr<Process> Process::Execute(string command, string argument1)
    {
        vector<string> arguments;

        arguments.push_back(argument1);

        return Execute(command, arguments);
    }

    unique_ptr<Process> Process::Execute(string command, string argument1, string argument2)
    {
        vector<string> arguments;

        arguments.reserve(2);
        arguments.push_back(argument1);
        arguments.push_back(argument2);

        return Execute(command, arguments);
    }

    unique_ptr<Process> Process::Execute(string command, string argument1, string argument2, string argument3)
    {
        vector<string> arguments;

        arguments.reserve(3);
        arguments.push_back(argument1);
        arguments.push_back(argument2);
        arguments.push_back(argument3);

        return Execute(command, arguments);
    }

    unique_ptr<Process> Process::Execute(string command, string argument1, string argument2, string argument3, string argument4)
    {
        vector<string> arguments;

        arguments.reserve(4);
        arguments.push_back(argument1);
        arguments.push_back(argument2);
        arguments.push_back(argument3);
        arguments.push_back(argument4);

        return Execute(command, arguments);
    }

    unique_ptr<Process> Process::Execute(string command, vector<string> &arguments)
    {
        unique_ptr<tuple<unique_ptr<Pipe>, unique_ptr<Pipe>>> stdInPipePair = Pipe::CreatePipePair();
        unique_ptr<Pipe> stdInPipeParent = std::move(std::get<1>(*stdInPipePair));
        unique_ptr<Pipe> stdInPipeChild = std::move(std::get<0>(*stdInPipePair));
        unique_ptr<tuple<unique_ptr<Pipe>, unique_ptr<Pipe>>> stdOutPipePair = Pipe::CreatePipePair();
        unique_ptr<Pipe> stdOutPipeParent = std::move(std::get<0>(*stdOutPipePair));
        unique_ptr<Pipe> stdOutPipeChild = std::move(std::get<1>(*stdOutPipePair));
        unique_ptr<tuple<unique_ptr<Pipe>, unique_ptr<Pipe>>> stdErrPipePair = Pipe::CreatePipePair();
        unique_ptr<Pipe> stdErrPipeParent = std::move(std::get<0>(*stdErrPipePair));
        unique_ptr<Pipe> stdErrPipeChild = std::move(std::get<1>(*stdErrPipePair));

        int processId = fork();

        if (processId < 0)
        {
            throw ProcessException("fork() returned an error");
        }

        if (processId > 0)
        {
            // Parent process.
            return unique_ptr<Process>(new Process(processId, std::move(stdInPipeParent), std::move(stdOutPipeParent), std::move(stdErrPipeParent)));
        }
        else
        {
            // Child process.
            stdInPipeParent = nullptr;
            stdOutPipeParent = nullptr;
            stdErrPipeParent = nullptr;

            // Redirect stdInData.
            if (dup2(stdInPipeChild->GetFileDescriptor(), STDIN_FILENO) == -1) {
                exit(errno);
            }

            // Redirect stdout.
            if (dup2(stdOutPipeChild->GetFileDescriptor(), STDOUT_FILENO) == -1) {
                exit(errno);
            }

            // Redirect stderr.
            if (dup2(stdErrPipeChild->GetFileDescriptor(), STDERR_FILENO) == -1) {
                exit(errno);
            }

            stdInPipeChild = nullptr;
            stdOutPipeChild = nullptr;
            stdErrPipeChild = nullptr;

            char *argumentArray[arguments.size() + 1];
            int argumentNumber = 0;

            for (vector<string>::const_iterator it = arguments.cbegin(); it != arguments.cend(); ++it)
            {
                argumentArray[argumentNumber++] = (char*) (*it).c_str();
            }

            argumentArray[argumentNumber] = nullptr;

            execvp(command.data(), argumentArray);
            exit(errno);
        }

        throw InvalidOperationException("Internal: This code should never execute");
    }

    int Process::GetProcessId()
    {
        return processId;
    }

    Pipe &Process::GetStandardIn()
    {
        return *standardIn;
    }

    Pipe &Process::GetStandardOut()
    {
        return *standardOut;
    }

    Pipe &Process::GetStandardError()
    {
        return *standardError;
    }

    void Process::Wait()
    {
        if (!hasExited)
        {
            waitpid(processId, &exitStatus, 0);
            hasExited = true;
        }
    }

    int Process::GetExitStatus()
    {
        if (!hasExited)
        {
            throw InvalidOperationException("Process exit has not been observed");
        }

        return exitStatus;
    }
}
