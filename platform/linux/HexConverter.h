#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Types.h"

namespace Sequent {
namespace HexConverter
{
    std::unique_ptr<uint8_vector> HexStringToByteArray(std::string hex);
    std::string ByteArrayToHexString(const uint8_vector &buffer);
    std::string ByteArrayToHexString(const uint8_vector &buffer, size_t offset, size_t count);
}
}

