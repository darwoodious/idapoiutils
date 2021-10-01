#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <unistd.h>
#include <poll.h>

#include "Types.h"
#include "IoException.h"
#include "ProcessException.h"
#include "Process.h"
#include "Pipe.h"
#include "ProcessResult.h"

#include "ProcessUtilities.h"

using std::string;
using std::unique_ptr;
using std::make_unique;
using std::vector;
using std::function;

namespace Sequent {
namespace ProcessUtilities
{
    unique_ptr<ProcessResult> ExecuteWaitReadOutputs(string command, uint8_vector &stdInData, string argument1)
    {
        vector<string> arguments;

        arguments.push_back(argument1);

        return ExecuteWaitReadOutputs(command, stdInData, arguments);
    }

    unique_ptr<ProcessResult> ExecuteWaitReadOutputs(string command, uint8_vector &stdInData, string argument1, string argument2)
    {
        vector<string> arguments;

        arguments.reserve(2);
        arguments.push_back(argument1);
        arguments.push_back(argument2);

        return ExecuteWaitReadOutputs(command, stdInData, arguments);
    }

    unique_ptr<ProcessResult> ExecuteWaitReadOutputs(string command, uint8_vector &stdInData, string argument1, string argument2, string argument3)
    {
        vector<string> arguments;

        arguments.reserve(3);
        arguments.push_back(argument1);
        arguments.push_back(argument2);
        arguments.push_back(argument3);

        return ExecuteWaitReadOutputs(command, stdInData, arguments);
    }

    unique_ptr<ProcessResult> ExecuteWaitReadOutputs(string command, uint8_vector &stdInData, string argument1, string argument2, string argument3, string argument4)
    {
        vector<string> arguments;

        arguments.reserve(4);
        arguments.push_back(argument1);
        arguments.push_back(argument2);
        arguments.push_back(argument3);
        arguments.push_back(argument4);

        return ExecuteWaitReadOutputs(command, stdInData, arguments);
    }

    unique_ptr<ProcessResult> ExecuteWaitReadOutputs(string command, uint8_vector &stdInData, vector<string> &arguments)
    {
        unique_ptr<Process> process = Process::Execute(command, arguments);

        unique_ptr<uint8_vector> stdOutDataPointer = make_unique<uint8_vector>();
        unique_ptr<uint8_vector> stdErrDataPointer = make_unique<uint8_vector>();
        uint8_vector &stdOutData(*stdOutDataPointer);
        uint8_vector &stdErrData(*stdErrDataPointer);
        pollfd pollFileDescriptors[3];
        size_t stdInDataPosition = 0;
        unique_ptr<uint8_vector> buffer = make_unique<uint8_vector>(8192);
        int fileDescriptorCount = stdInData.size() ? 3 : 2;
        Pipe &stdIn = process->GetStandardIn();
        Pipe &stdOut = process->GetStandardOut();
        Pipe &stdErr = process->GetStandardError();

        if (stdInDataPosition == stdInData.size())
        {
            stdIn.Close();
        }

        pollFileDescriptors[0].fd = stdInDataPosition < stdInData.size() ? stdIn.GetFileDescriptor() : -1;
        pollFileDescriptors[0].events = POLLOUT;
        pollFileDescriptors[1].fd = stdOut.GetFileDescriptor();
        pollFileDescriptors[1].events = POLLIN;
        pollFileDescriptors[2].fd = stdErr.GetFileDescriptor();
        pollFileDescriptors[2].events = POLLIN;

        while (fileDescriptorCount)
        {
            poll(pollFileDescriptors, 3, -1);

            if (pollFileDescriptors[0].revents & POLLOUT)
            {
                stdInDataPosition += write(stdIn.GetFileDescriptor(), stdInData.data() + stdInDataPosition, std::min((size_t) 8192, stdInData.size() - stdInDataPosition));

                if (stdInDataPosition == stdInData.size())
                {
                    pollFileDescriptors[0].fd = -1;
                    --fileDescriptorCount;
                    stdIn.Close();
                }
            }

            if (pollFileDescriptors[1].revents & POLLIN)
            {
                stdOutData.insert(stdOutData.end(), buffer->cbegin(), buffer->cbegin() + stdOut.Read(*buffer, 0, 8192));
            }

            if (pollFileDescriptors[1].revents & POLLHUP)
            {
                pollFileDescriptors[1].fd = -1;
                --fileDescriptorCount;
                stdOut.Close();
            }

            if (pollFileDescriptors[2].revents & POLLIN)
            {
                stdErrData.insert(stdErrData.end(), buffer->cbegin(), buffer->cbegin() + stdErr.Read(*buffer, 0, 8192));
            }

            if (pollFileDescriptors[2].revents & POLLHUP)
            {
                pollFileDescriptors[2].fd = -1;
                --fileDescriptorCount;
                stdErr.Close();
            }
        }

        process->Wait();

        return make_unique<ProcessResult>(WIFEXITED(process->GetExitStatus()) ? WEXITSTATUS(process->GetExitStatus()) : 255, std::move(stdOutDataPointer), std::move(stdErrDataPointer));
    }
}
}
