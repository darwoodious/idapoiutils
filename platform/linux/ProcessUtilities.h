#pragma once

#include <string>
#include <memory>

#include "Types.h"
#include "Process.h"
#include "ProcessResult.h"

namespace Sequent {
namespace ProcessUtilities
{
    std::unique_ptr<ProcessResult> ExecuteWaitReadOutputs(std::string command, uint8_vector &stdInData, std::string argument1);
    std::unique_ptr<ProcessResult> ExecuteWaitReadOutputs(std::string command, uint8_vector &stdInData, std::string argument1, std::string argument2);
    std::unique_ptr<ProcessResult> ExecuteWaitReadOutputs(std::string command, uint8_vector &stdInData, std::string argument1, std::string argument2, std::string argument3);
    std::unique_ptr<ProcessResult> ExecuteWaitReadOutputs(std::string command, uint8_vector &stdInData, std::string argument1, std::string argument2, std::string argument3, std::string argument4);
    std::unique_ptr<ProcessResult> ExecuteWaitReadOutputs(std::string command, uint8_vector &stdInData, std::vector<std::string> &arguments);
}
}
