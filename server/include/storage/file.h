#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>

class File
{
public:
    using Path = std::filesystem::path;

    enum class Mode
    {
        Read,
        Append,
    };

    enum class ReadStatus
    {
        Ok,
        MoreData,
        Error,
    };

    struct ReadResult
    {
        ReadStatus  status;
        std::size_t n;
    };

    enum class WriteStatus
    {
        Ok,
        Error,
    };

    struct WriteResult
    {
        WriteStatus status;
        std::size_t n;
    };

    static std::optional<File> open(const Path &path, Mode mode);

    ReadResult  read(std::uint64_t offset, char *buf, std::size_t max);
    WriteResult append(const char *data, std::size_t n);

private:
    File(Path path, std::fstream stream);

    Path         path;
    std::fstream stream;
};
