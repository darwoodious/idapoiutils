#include <memory>
#include <string>

#include "Types.h"
#include "ArgumentOutOfRangeException.h"

#include "AsciiEncoder.h"

using std::unique_ptr;
using std::make_unique;
using std::string;

namespace Sequent {
namespace AsciiEncoder
{
    static void EncodeInternal(string &asciiString, uint8_vector &buffer, size_t asciiOffset, size_t bufferOffset, size_t count);
    static void DecodeInternal(uint8_vector &buffer, string &asciiString, size_t offset, size_t count);
        
    unique_ptr<uint8_vector> Encode(string asciiString)
    {
        unique_ptr<uint8_vector> bufferPointer = make_unique<uint8_vector>();

        bufferPointer->reserve(asciiString.size());

        for (string::const_iterator it = asciiString.begin(); it != asciiString.end(); ++it)
        {
            bufferPointer->push_back(*it);
        }

        return bufferPointer;
    }

    void Encode(string asciiString, uint8_vector &buffer, size_t asciiOffset, size_t bufferOffset, size_t count)
    {
        if (asciiOffset > 0 && asciiOffset >= asciiString.size())
        {
            throw ArgumentOutOfRangeException("asciiOffset");
        }

        if (asciiOffset + count > asciiString.size())
        {
            throw ArgumentOutOfRangeException("count");
        }

        if (bufferOffset > 0 && bufferOffset >= buffer.size())
        {
            throw ArgumentOutOfRangeException("bufferOffset");
        }

        if (bufferOffset + count > buffer.size())
        {
            throw ArgumentOutOfRangeException("count");
        }

        EncodeInternal(asciiString, buffer, asciiOffset, bufferOffset, count);
    }

    static void EncodeInternal(string &asciiString, uint8_vector &buffer, size_t asciiOffset, size_t bufferOffset, size_t count)
    {
        while (count-- > 0)
        {
            buffer[bufferOffset++] = asciiString[asciiOffset++];
        }
    }

    string Decode(uint8_vector &buffer)
    {
        string asciiString;

        asciiString.reserve(buffer.size());
        DecodeInternal(buffer, asciiString, 0, buffer.size());

        return asciiString;
    }

    string Decode(uint8_vector &buffer, size_t offset, size_t count)
    {
        if (offset > 0 && offset >= buffer.size())
        {
            throw ArgumentOutOfRangeException("offset");
        }

        if (offset + count > buffer.size())
        {
            throw ArgumentOutOfRangeException("count");
        }

        string asciiString;

        asciiString.reserve(count);
        DecodeInternal(buffer, asciiString, offset, count);

        return asciiString;
    }

    static void DecodeInternal(uint8_vector &buffer, string &asciiString, size_t offset, size_t count)
    {
        for (uint8_vector::const_iterator it = buffer.cbegin() + offset; count-- > 0 && it != buffer.cend(); ++it)
        {
            uint8_t b = *it;

            if (b < 0x07)
            {
                asciiString.push_back('?');
            }
            else if (b <= 0x0D)
            {
                asciiString.push_back((const char) b);
            }
            else if (b < 0x20)
            {
                asciiString.push_back('?');
            }
            else if (b <= 0x7E)
            {
                asciiString.push_back((const char) b);
            }
            else
            {
                asciiString.push_back('?');
            }
        }
    }
}
}