#include <memory>
#include <vector>
#include <string>

#include "Types.h"
#include "ArgumentOutOfRangeException.h"

#include "HexConverter.h"

using std::unique_ptr;
using std::make_unique;
using std::string;

namespace Sequent {
namespace HexConverter
{
    // Hex alphabet lookup table, used to convert nibbles to hex characters.
    static const char *nibbleToHexCharacterTable = "0123456789ABCDEF";

    // Hex alphabet lookup table, used to convert hex characters to nibbles.
    // This is a portion (zero-based) of the ASCII character table.
    // Each byte in the table is either the index (zero-based) within the hexadecimal character set
    // of the valid ASCII character that it corresponds to, or -1 signifying an invalid ASCII byte.
    static uint8_t hexCharacterToNibbleTable[] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // < '0' (invalid)
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // < '0' (invalid)
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // < '0' (invalid)
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // < '0' (invalid)
        0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9,                       // '0' - '9' (valid)
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                               // ':' - '@' (invalid)
        0xA, 0xB, 0xC, 0xD, 0xE, 0xF,                                           // 'A' - 'F' (valid)
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,             // 'G' - 'P' (invalid)
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,             // 'Q' - 'Z' (invalid)
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                                     // '[' - '`' (invalid)
        0xA, 0xB, 0xC, 0xD, 0xE, 0xF                                            // 'a' - 'f' (valid)
    };

    static void ByteArrayToHexStringInternal(const uint8_vector &buffer, size_t bufferOffset, string &hex, size_t hexOffset, size_t byteCount);
    static void HexStringToByteArrayInternal(string &hex, size_t hexOffset, uint8_vector &buffer, size_t bufferOffser, size_t count);

    string ByteArrayToHexString(const uint8_vector &buffer)
    {
        string hex = string(buffer.size() * 2, 0);

        ByteArrayToHexStringInternal(buffer, 0, hex, 0, buffer.size());

        return hex;
    }

    string ByteArrayToHexString(const uint8_vector &buffer, size_t offset, size_t count)
    {
        if (offset > 0 && offset >= buffer.size())
        {
            throw ArgumentOutOfRangeException("offset");
        }

        if (count > 0 && offset + count > buffer.size())
        {
            throw ArgumentOutOfRangeException("count");
        }

        string hex = string(count * 2, 0);

        ByteArrayToHexStringInternal(buffer, offset, hex, 0, count);

        return hex;
    }

    static void ByteArrayToHexStringInternal(const uint8_vector &buffer, size_t bufferOffset, string &hex, size_t hexOffset, size_t byteCount)
    {
        for (; byteCount > 0; --byteCount)
        {
            uint8_t b = buffer[bufferOffset++];

            hex[hexOffset++] = nibbleToHexCharacterTable[b >> 4];
            hex[hexOffset++] = nibbleToHexCharacterTable[b & 0xF];
        }
    }

    unique_ptr<uint8_vector> HexStringToByteArray(string hex)
    {
        if (hex.size() % 2 != 0)
        {
            throw ArgumentOutOfRangeException("Hex string must contain even number of characters");
        }

        unique_ptr<uint8_vector> bufferPointer = make_unique<uint8_vector>(hex.size() / 2);

        HexStringToByteArrayInternal(hex, 0, *bufferPointer, 0, bufferPointer->size());

        return bufferPointer;
    }

    static void HexStringToByteArrayInternal(string &hex, size_t hexOffset, uint8_vector &buffer, size_t bufferOffset, size_t byteCount)
    {
        for (; byteCount > 0; --byteCount)
        {
            int highNibbleOffset = hex[hexOffset++];
            int lowNibbleOffset = hex[hexOffset++];
            uint8_t highNibble;
            uint8_t lowNibble;

            if (
                highNibbleOffset < 0 ||
                highNibbleOffset >= sizeof(hexCharacterToNibbleTable) ||
                lowNibbleOffset < 0 ||
                lowNibbleOffset >= sizeof(hexCharacterToNibbleTable) ||
                (highNibble = hexCharacterToNibbleTable[highNibbleOffset]) == 0xFF ||
                (lowNibble = hexCharacterToNibbleTable[lowNibbleOffset]) == 0xFF
            )
            {
                throw ArgumentOutOfRangeException("Hex encoded data invalid");
            }

            buffer[bufferOffset++] = (uint8_t) ((highNibble << 4) | lowNibble);
        }
    }
}
}

