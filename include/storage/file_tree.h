#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "storage/directory.h"

class FileTree
{
public:
    using Path = std::filesystem::path;

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

    static constexpr std::size_t block_size = 4096;

    explicit FileTree(const Path &root_path);

    bool scan();
    void print() const;
    std::optional<std::vector<std::string>> list(const Path &rel) const;

    // TODO: cache open fds instead of reopening the file on every read.
    ReadResult read(const Path &rel, std::uint64_t offset, char *buf, std::size_t max);

    WriteResult write(const Path &rel, const char *data, std::size_t n);

private:
    // TODO: path can still be swapped between resolve() and open;
    // Shouldnt we use openat2(RESOLVE_BENEATH) on Linux?
    std::optional<Path> resolve(const Path &rel) const;

    Path      root_path;
    Directory root;
};
