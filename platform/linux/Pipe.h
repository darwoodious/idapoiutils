#pragma once

#include <memory>

#include "Types.h"

namespace Sequent
{
    enum struct PipeDirection
    {
        Read,
        Write
    };

    class Pipe
    {
    private:
        int fileDescriptor;
        PipeDirection pipeDirection;

        void AssertNotClosed();

    public:
        Pipe(int fileDescriptor, PipeDirection pipeDirection);
        ~Pipe();

        static std::unique_ptr<std::tuple<std::unique_ptr<Pipe>, std::unique_ptr<Pipe>>> CreatePipePair();

        int GetFileDescriptor();
        size_t Read(uint8_vector &buffer, size_t offset, size_t count);
        size_t Write(uint8_vector &buffer, size_t offset, size_t count);
        void Close();
    };
}
