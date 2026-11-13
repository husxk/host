#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "storage/file.h"

struct FileInfo
{
    enum class Type
    {
        File,
        Directory,
    };

    Type                                  type;
    std::uint64_t                         size;
    std::chrono::system_clock::time_point mtime;
};

class FileTree
{
public:
    using Path = std::filesystem::path;

    using ReadStatus  = File::ReadStatus;
    using ReadResult  = File::ReadResult;
    using WriteStatus = File::WriteStatus;
    using WriteResult = File::WriteResult;

    static constexpr std::size_t block_size = 4096;

    explicit FileTree(const Path &root_path);

    std::optional<std::vector<std::string>> print() const;
    std::optional<std::vector<std::string>> list(const Path &rel) const;
    std::optional<FileInfo>                 stat(const Path &rel) const;

    bool mkdir(const Path &rel);

    // TODO: dst can appear between the exists check and rename; use renameat2(RENAME_NOREPLACE) on Linux?
    bool mv(const Path &src, const Path &dst);

    // TODO: cache open fds instead of reopening the file on every read.
    ReadResult read(const Path &rel, std::uint64_t offset, char *buf, std::size_t max);

    WriteResult write(const Path &rel, const char *data, std::size_t n);

private:
    struct Entry
    {
        std::string name;
        bool        is_dir;
        bool        is_symlink;
    };

    // TODO: path can still be swapped between resolve() and open;
    // Shouldnt we use openat2(RESOLVE_BENEATH) on Linux?
    std::optional<Path> resolve(const Path &rel) const;
    std::optional<Path> resolve_entry(const Path &rel) const;

    std::optional<std::vector<Entry>> entries(const Path &rel) const;

    bool walk(const Path &rel, int depth, std::vector<std::string> &lines) const;

    Path root_path;
};
