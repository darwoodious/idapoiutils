#pragma once

#include <memory>
#include <string>

#include "Types.h"

namespace Sequent
{
    class ProcessResult
    {
    private:
        int status;
        std::unique_ptr<uint8_vector> stdOut;
        std::unique_ptr<uint8_vector> stdErr;

    public:
        ProcessResult(int status, std::unique_ptr<uint8_vector> stdOut, std::unique_ptr<uint8_vector> stdErr);

        int GetStatus() const;
        uint8_vector &GetStdOut() const;
        uint8_vector &GetStdErr() const;
    };
}
