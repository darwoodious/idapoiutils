#include <memory>
#include <unistd.h>

#include "Types.h"
#include "ArgumentOutOfRangeException.h"
#include "InvalidOperationException.h"
#include "IoException.h"
#include "EndOfStreamException.h"

#include "Pipe.h"

using std::unique_ptr;
using std::make_unique;
using std::tuple;

namespace Sequent
{
    Pipe::Pipe(int fileDescriptor, PipeDirection pipeDirection)
    : fileDescriptor(fileDescriptor), pipeDirection(pipeDirection)
    { }

    Pipe::~Pipe()
    {
        if (fileDescriptor > 0)
        {
            close(fileDescriptor);
        }
    }

    tuple<unique_ptr<Pipe>, unique_ptr<Pipe>> Pipe::CreatePipePair()
    {
        int pipePair[2];

        if (pipe(pipePair) < 0) {
            throw IoException("Failed to create pipe pair");
        }

        return make_tuple(
            make_unique<Pipe>(pipePair[0], PipeDirection::Read),
            make_unique<Pipe>(pipePair[1], PipeDirection::Write)
        );
    }

    int Pipe::GetFileDescriptor()
    {
        AssertNotClosed();

        return fileDescriptor;
    }

    size_t Pipe::Read(uint8_vector &buffer, size_t offset, size_t count)
    {
        AssertNotClosed();

        if (pipeDirection != PipeDirection::Read)
        {
            throw InvalidOperationException("Cannot read from a write pipe");
        }

        if (offset > 0 && offset >= buffer.size())
        {
            throw ArgumentOutOfRangeException("offset");
        }

        if (offset + count > buffer.size())
        {
            throw ArgumentOutOfRangeException("count");
        }

        int bytesRead = read(fileDescriptor, buffer.data() + offset, count);

        if (bytesRead == -1)
        {
            throw IoException();
        }

        return bytesRead;
    }

    size_t Pipe::Write(uint8_vector &buffer, size_t offset, size_t count)
    {
        AssertNotClosed();

        if (pipeDirection != PipeDirection::Write)
        {
            throw InvalidOperationException("Cannot write to a read pipe");
        }

        if (offset > 0 && offset >= buffer.size())
        {
            throw ArgumentOutOfRangeException("offset");
        }

        if (offset + count > buffer.size())
        {
            throw ArgumentOutOfRangeException("count");
        }

        return write(fileDescriptor, buffer.data() + offset, count);
    }

    void Pipe::Close()
    {
        int fileDescriptor = this->fileDescriptor;

        this->fileDescriptor = 0;

        if (fileDescriptor > 0)
        {
            close(fileDescriptor);
        }
    }

    void Pipe::AssertNotClosed()
    {
        if (fileDescriptor == 0)
        {
            throw InvalidOperationException("Pipe closed");
        }
    }
}

