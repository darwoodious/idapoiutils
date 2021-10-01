#include <memory>
#include <string>

#include "Types.h"

#include "ProcessResult.h"

using std::unique_ptr;
using std::string;

namespace Sequent
{
    ProcessResult::ProcessResult(int status, unique_ptr<uint8_vector> stdOut, unique_ptr<uint8_vector> stdErr)
    : status(status), stdOut(std::move(stdOut)), stdErr(std::move(stdErr))
    { }

    int ProcessResult::GetStatus() const
    {
        return status;
    }

    uint8_vector &ProcessResult::GetStdOut() const
    {
        return *stdOut;
    }

    uint8_vector &ProcessResult::GetStdErr() const
    {
        return *stdErr;
    }
}
