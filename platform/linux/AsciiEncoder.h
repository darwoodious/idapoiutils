#pragma once

#include <memory>
#include <string>

#include "Types.h"

namespace Sequent {
namespace AsciiEncoder
{
    std::unique_ptr<uint8_vector> Encode(std::string asciiString);
    void Encode(std::string asciiString, uint8_vector &buffer, size_t asciiOffset, size_t bufferOffset, size_t count);
    std::string Decode(uint8_vector &buffer);
    std::string Decode(uint8_vector &buffer, size_t offset, size_t count);
}
}

