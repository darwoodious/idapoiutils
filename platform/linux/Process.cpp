#include <memory>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <stdarg.h>
#include <errno.h>

#include "Types.h"
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
    Process::Process(int processId, Redirect redirect, shared_ptr<Pipe> standardIn, shared_ptr<Pipe> standardOut, shared_ptr<Pipe> standardError)
    : processId(processId), redirect(redirect), standardIn(standardIn), standardOut(standardOut), standardError(standardError), hasExited(false)
    { }

    unique_ptr<Process> Process::Execute(string command, Redirect redirect, string argument1)
    {
        vector<string> arguments;

        arguments.reserve(1);
        arguments.push_back(argument1);

        return Execute(command, redirect, arguments);
    }

    unique_ptr<Process> Process::Execute(string command, Redirect redirect, string argument1, string argument2)
    {
        vector<string> arguments;

        arguments.reserve(2);
        arguments.push_back(argument1);
        arguments.push_back(argument2);

        return Execute(command, redirect, arguments);
    }

    unique_ptr<Process> Process::Execute(string command, Redirect redirect, string argument1, string argument2, string argument3)
    {
        vector<string> arguments;

        arguments.reserve(3);
        arguments.push_back(argument1);
        arguments.push_back(argument2);
        arguments.push_back(argument3);

        return Execute(command, redirect, arguments);
    }

    unique_ptr<Process> Process::Execute(string command, Redirect redirect, string argument1, string argument2, string argument3, string argument4)
    {
        vector<string> arguments;

        arguments.reserve(4);
        arguments.push_back(argument1);
        arguments.push_back(argument2);
        arguments.push_back(argument3);
        arguments.push_back(argument4);

        return Execute(command, redirect, arguments);
    }

    unique_ptr<Process> Process::Execute(string command, Redirect redirect, vector<string> &arguments)
    {
        unique_ptr<Pipe> stdInPipeParent;
        unique_ptr<Pipe> stdInPipeChild;
        unique_ptr<Pipe> stdOutPipeParent;
        unique_ptr<Pipe> stdOutPipeChild;
        unique_ptr<Pipe> stdErrPipeParent;
        unique_ptr<Pipe> stdErrPipeChild;

        if ((redirect & Redirect::StandardIn) == Redirect::StandardIn)
        {
            tuple<unique_ptr<Pipe>, unique_ptr<Pipe>> stdInPipePair = Pipe::CreatePipePair();
            stdInPipeParent = std::move(std::get<1>(stdInPipePair));
            stdInPipeChild = std::move(std::get<0>(stdInPipePair));
        }

        if ((redirect & Redirect::StandardOut) == Redirect::StandardOut)
        {
            tuple<unique_ptr<Pipe>, unique_ptr<Pipe>> stdOutPipePair = Pipe::CreatePipePair();
            stdOutPipeParent = std::move(std::get<0>(stdOutPipePair));
            stdOutPipeChild = std::move(std::get<1>(stdOutPipePair));
        }

        if ((redirect & Redirect::StandardError) == Redirect::StandardError)
        {
            tuple<unique_ptr<Pipe>, unique_ptr<Pipe>> stdErrPipePair = Pipe::CreatePipePair();
            stdErrPipeParent = std::move(std::get<0>(stdErrPipePair));
            stdErrPipeChild = std::move(std::get<1>(stdErrPipePair));
        }

        int processId = fork();

        if (processId < 0)
        {
            throw ProcessException("fork() returned an error");
        }

        if (processId > 0)
        {
            // Parent process.
            return unique_ptr<Process>(new Process(processId, redirect, std::move(stdInPipeParent), std::move(stdOutPipeParent), std::move(stdErrPipeParent)));
        }
        else
        {
            // Child process.
            stdInPipeParent = nullptr;
            stdOutPipeParent = nullptr;
            stdErrPipeParent = nullptr;

            if ((redirect & Redirect::StandardIn) == Redirect::StandardIn)
            {
                if (dup2(stdInPipeChild->GetFileDescriptor(), STDIN_FILENO) == -1) {
                    exit(errno);
                }
            }

            if ((redirect & Redirect::StandardOut) == Redirect::StandardOut)
            {
                if (dup2(stdOutPipeChild->GetFileDescriptor(), STDOUT_FILENO) == -1) {
                    exit(errno);
                }
            }

            if ((redirect & Redirect::StandardError) == Redirect::StandardError)
            {
                if (dup2(stdErrPipeChild->GetFileDescriptor(), STDERR_FILENO) == -1) {
                    exit(errno);
                }
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

    Redirect Process::GetRedirect()
    {
        return redirect;
    }

    Pipe &Process::GetStandardIn()
    {
        if ((redirect & Redirect::StandardIn) != Redirect::StandardIn)
        {
            throw InvalidOperationException("Standard In was not redirected");
        }

        return *standardIn;
    }

    Pipe &Process::GetStandardOut()
    {
        if ((redirect & Redirect::StandardOut) != Redirect::StandardOut)
        {
            throw InvalidOperationException("Standard Out was not redirected");
        }

        return *standardOut;
    }

    Pipe &Process::GetStandardError()
    {
        if ((redirect & Redirect::StandardError) != Redirect::StandardError)
        {
            throw InvalidOperationException("Standard Error was not redirected");
        }

        return *standardError;
    }

    void Process::ExchangeStandardIo(uint8_vector *standardInData, uint8_vector *standardOutData, uint8_vector *standardErrorData)
    {
        if (redirect == Redirect::None)
        {
            return;
        }

        size_t fileDescriptorCount = 0;
        Pipe *standardInPipe;
        Pipe *standardOutPipe;
        Pipe *standardErrorPipe;

        if ((redirect & Redirect::StandardIn) == Redirect::StandardIn)
        {
            standardInPipe = &*standardIn;

            if (!standardInData || !standardInData->size())
            {
                standardInPipe->Close();
                standardInPipe = nullptr;
            }
            else
            {
                ++fileDescriptorCount;
            }
        }

        if ((redirect & Redirect::StandardOut) == Redirect::StandardOut)
        {
            standardOutPipe = &*standardOut;
            ++fileDescriptorCount;
        }

        if ((redirect & Redirect::StandardError) == Redirect::StandardError)
        {
            standardErrorPipe = &*standardError;
            ++fileDescriptorCount;
        }

        if (!fileDescriptorCount)
        {
            return;
        }

        pollfd pollFileDescriptors[3];
        size_t standardInPosition = 0;
        unique_ptr<uint8_vector> readBuffer;

        if (standardOutPipe || standardErrorPipe)
        {
            readBuffer = make_unique<uint8_vector>(8192);
        }

        pollFileDescriptors[0].fd = standardInPipe ? standardInPipe->GetFileDescriptor() : -1;
        pollFileDescriptors[0].events = POLLOUT;
        pollFileDescriptors[1].fd = standardOutPipe ? standardOutPipe->GetFileDescriptor() : -1;
        pollFileDescriptors[1].events = POLLIN;
        pollFileDescriptors[2].fd = standardErrorPipe ? standardErrorPipe->GetFileDescriptor() : -1;
        pollFileDescriptors[2].events = POLLIN;

        while (fileDescriptorCount)
        {
            poll(pollFileDescriptors, 3, -1);

            if (pollFileDescriptors[0].revents & POLLOUT)
            {
                standardInPosition += standardInPipe->Write(*standardInData, standardInPosition, std::min((size_t) 8192, standardInData->size() - standardInPosition));

                if (standardInPosition == standardInData->size())
                {
                    pollFileDescriptors[0].fd = -1;
                    --fileDescriptorCount;
                    standardInPipe->Close();
                }
            }

            if (pollFileDescriptors[1].revents & POLLIN)
            {
                size_t bytesRead = standardOutPipe->Read(*readBuffer, 0, readBuffer->size());

                if (standardOutData)
                {
                    standardOutData->insert(standardOutData->end(), readBuffer->cbegin(), readBuffer->cbegin() + bytesRead);
                }
            }

            if (pollFileDescriptors[1].revents & POLLHUP)
            {
                pollFileDescriptors[1].fd = -1;
                --fileDescriptorCount;
                standardOutPipe->Close();
            }

            if (pollFileDescriptors[2].revents & POLLIN)
            {
                size_t bytesRead = standardErrorPipe->Read(*readBuffer, 0, readBuffer->size());

                if (standardErrorData)
                {
                    standardErrorData->insert(standardErrorData->end(), readBuffer->cbegin(), readBuffer->cbegin() + bytesRead);
                }
            }

            if (pollFileDescriptors[2].revents & POLLHUP)
            {
                pollFileDescriptors[2].fd = -1;
                --fileDescriptorCount;
                standardErrorPipe->Close();
            }
        }
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
