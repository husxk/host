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

    static constexpr std::size_t block_size = 4096;

    explicit FileTree(const Path &root_path);

    bool scan();
    void print() const;
    std::optional<std::vector<std::string>> list(const Path &rel) const;

    // TODO: cache open fds instead of reopening the file on every read.
    ReadResult read(const Path &rel, std::uint64_t offset, char *buf, std::size_t max);

private:
    Path      root_path;
    Directory root;
};
