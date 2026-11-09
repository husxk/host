#include "storage/file_tree.h"

#include <algorithm>
#include <fstream>
#include <system_error>

#include "logger.h"

FileTree::FileTree(const Path &root_path)
    : root_path(root_path)
    , root("")
{
}

bool FileTree::scan()
{
    return this->root.scan(this->root_path);
}

void FileTree::print() const
{
    std::error_code ec;
    const Path      abs_path = std::filesystem::absolute(this->root_path, ec);

    // Debug log
    // TODO: remove it later.
    Logger::plain("[debug] %s", ec ? this->root_path.c_str() : abs_path.c_str());

    this->root.print();
}

std::optional<FileTree::Path> FileTree::resolve(const Path &rel) const
{
    std::error_code ec;
    const Path      root = std::filesystem::canonical(this->root_path, ec);

    if (ec)
    {
        Logger::error("file_tree: resolve root %s: %s", this->root_path.c_str(), ec.message().c_str());
        return std::nullopt;
    }

    const Path path = std::filesystem::weakly_canonical(root / rel.relative_path(), ec);

    if (ec)
    {
        return std::nullopt;
    }

    const auto mismatch = std::mismatch(root.begin(), root.end(), path.begin(), path.end());

    if (mismatch.first != root.end())
    {
        return std::nullopt;
    }

    // weakly_canonical leaves dangling symlinks unresolved; they could point outside the root.
    Path prefix;

    for (const Path &part : path)
    {
        prefix /= part;

        if (std::filesystem::is_symlink(std::filesystem::symlink_status(prefix, ec)))
        {
            return std::nullopt;
        }
    }

    return path;
}

std::optional<std::vector<std::string>> FileTree::list(const Path &rel) const
{
    const std::optional<Path> path = this->resolve(rel);

    if (!path)
    {
        return std::nullopt;
    }

    std::error_code                           ec;
    std::filesystem::directory_iterator       it(*path, ec);
    const std::filesystem::directory_iterator end;

    if (ec)
    {
        return std::nullopt;
    }

    std::vector<std::string> names;

    for (; it != end; it.increment(ec))
    {
        const std::string name = it->path().filename().string();

        if (it->is_symlink(ec) && !this->resolve(rel / name))
        {
            continue;
        }

        const std::filesystem::file_status status = it->status(ec);

        if (ec)
        {
            continue;
        }

        if (std::filesystem::is_directory(status))
        {
            names.push_back(name + "/");
        }
        else if (std::filesystem::is_regular_file(status))
        {
            names.push_back(name);
        }
    }

    if (ec)
    {
        Logger::error("file_tree: list %s: %s", path->c_str(), ec.message().c_str());
        return std::nullopt;
    }

    std::sort(names.begin(), names.end());

    return names;
}

FileTree::ReadResult FileTree::read(const Path    &rel,
                                    std::uint64_t  offset,
                                    char          *buf,
                                    std::size_t    max)
{
    const std::optional<Path> path = this->resolve(rel);
    std::error_code           ec;

    if (!path || !std::filesystem::is_regular_file(*path, ec))
    {
        return {ReadStatus::Error, 0};
    }

    std::ifstream in(*path, std::ios::binary);

    if (!in)
    {
        Logger::error("file_tree: open %s failed", path->c_str());
        return {ReadStatus::Error, 0};
    }

    const std::size_t count = std::min(max, FileTree::block_size);

    in.seekg(static_cast<std::streamoff>(offset));
    in.read(buf, static_cast<std::streamsize>(count));

    if (in.bad())
    {
        Logger::error("file_tree: read %s failed", path->c_str());
        return {ReadStatus::Error, 0};
    }

    const std::size_t n   = static_cast<std::size_t>(in.gcount());
    const bool        eof = in.peek() == std::ifstream::traits_type::eof();

    return {eof ? ReadStatus::Ok : ReadStatus::MoreData, n};
}

FileTree::WriteResult FileTree::write(const Path &rel, const char *data, std::size_t n)
{
    const std::optional<Path> path = this->resolve(rel);

    if (!path)
    {
        return {WriteStatus::Error, 0};
    }

    std::error_code                    ec;
    const std::filesystem::file_status status = std::filesystem::status(*path, ec);

    if (!std::filesystem::is_directory(path->parent_path(), ec)
        || (std::filesystem::exists(status) && !std::filesystem::is_regular_file(status)))
    {
        return {WriteStatus::Error, 0};
    }

    std::ofstream out(*path, std::ios::binary | std::ios::app);

    if (!out)
    {
        Logger::error("file_tree: open %s failed", path->c_str());
        return {WriteStatus::Error, 0};
    }

    out.write(data, static_cast<std::streamsize>(n));
    out.flush();

    if (!out)
    {
        Logger::error("file_tree: write %s failed", path->c_str());
        return {WriteStatus::Error, 0};
    }

    return {WriteStatus::Ok, n};
}
