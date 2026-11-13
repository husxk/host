#include "storage/file_tree.h"

#include <algorithm>
#include <system_error>

#include "logger.h"

FileTree::FileTree(const Path &root_path)
    : root_path(root_path)
{
}

std::optional<std::vector<std::string>> FileTree::print() const
{
    std::vector<std::string> lines {"/"};

    if (!this->walk(Path {}, 1, lines))
    {
        return std::nullopt;
    }

    return lines;
}

bool FileTree::walk(const Path &rel, int depth, std::vector<std::string> &lines) const
{
    const std::optional<std::vector<Entry>> found = this->entries(rel);

    if (!found)
    {
        return false;
    }

    const std::string indent(static_cast<std::size_t>(depth) * 2, ' ');

    for (const Entry &entry : *found)
    {
        lines.push_back(indent + (entry.is_dir ? entry.name + "/" : entry.name));

        if (entry.is_dir && !entry.is_symlink)
        {
            this->walk(rel / entry.name, depth + 1, lines);
        }
    }

    return true;
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

std::optional<FileTree::Path> FileTree::resolve_entry(const Path &rel) const
{
    Path entry = rel.relative_path();

    if (!entry.has_filename())
    {
        entry = entry.parent_path();
    }

    const Path name = entry.filename();

    if (name.empty() || name == "." || name == "..")
    {
        return std::nullopt;
    }

    const std::optional<Path> parent = this->resolve(entry.parent_path());

    if (!parent)
    {
        return std::nullopt;
    }

    return *parent / name;
}

std::optional<std::vector<FileTree::Entry>> FileTree::entries(const Path &rel) const
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

    std::vector<Entry> result;

    for (; it != end; it.increment(ec))
    {
        const std::string name       = it->path().filename().string();
        const bool        is_symlink = it->is_symlink(ec);

        if (is_symlink && !this->resolve(rel / name))
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
            result.push_back({name, true, is_symlink});
        }
        else if (std::filesystem::is_regular_file(status))
        {
            result.push_back({name, false, is_symlink});
        }
    }

    if (ec)
    {
        Logger::error("file_tree: list %s: %s", path->c_str(), ec.message().c_str());
        return std::nullopt;
    }

    std::sort(result.begin(), result.end(), [](const Entry &a, const Entry &b) { return a.name < b.name; });

    return result;
}

std::optional<FileInfo> FileTree::stat(const Path &rel) const
{
    const std::optional<Path> path = this->resolve(rel);

    if (!path)
    {
        return std::nullopt;
    }

    std::error_code                    ec;
    const std::filesystem::file_status status = std::filesystem::status(*path, ec);

    FileInfo info {};

    if (std::filesystem::is_regular_file(status))
    {
        info.type = FileInfo::Type::File;
        info.size = std::filesystem::file_size(*path, ec);

        if (ec)
        {
            Logger::error("file_tree: stat %s: %s", path->c_str(), ec.message().c_str());
            return std::nullopt;
        }
    }
    else if (std::filesystem::is_directory(status))
    {
        info.type = FileInfo::Type::Directory;
        info.size = 0;
    }
    else
    {
        return std::nullopt;
    }

    const std::filesystem::file_time_type mtime = std::filesystem::last_write_time(*path, ec);

    if (ec)
    {
        Logger::error("file_tree: stat %s: %s", path->c_str(), ec.message().c_str());
        return std::nullopt;
    }

    info.mtime = std::chrono::clock_cast<std::chrono::system_clock>(mtime);

    return info;
}

bool FileTree::mkdir(const Path &rel)
{
    const std::optional<Path> path = this->resolve(rel);

    if (!path)
    {
        return false;
    }

    std::error_code ec;
    const bool      created = std::filesystem::create_directory(*path, ec);

    if (ec)
    {
        Logger::error("file_tree: mkdir %s: %s", path->c_str(), ec.message().c_str());
        return false;
    }

    return created;
}

bool FileTree::mv(const Path &src, const Path &dst)
{
    const std::optional<Path> src_path = this->resolve_entry(src);
    std::optional<Path>       dst_path = this->resolve(dst);

    if (!src_path || !dst_path)
    {
        return false;
    }

    std::error_code ec;

    if (!std::filesystem::exists(std::filesystem::symlink_status(*src_path, ec)))
    {
        return false;
    }

    if (std::filesystem::is_directory(*dst_path, ec))
    {
        *dst_path /= src_path->filename();
    }

    if (std::filesystem::exists(std::filesystem::symlink_status(*dst_path, ec)))
    {
        return false;
    }

    std::filesystem::rename(*src_path, *dst_path, ec);

    if (ec)
    {
        Logger::error("file_tree: mv %s -> %s: %s", src_path->c_str(), dst_path->c_str(), ec.message().c_str());
        return false;
    }

    return true;
}

bool FileTree::rm(const Path &rel)
{
    const std::optional<Path> path = this->resolve_entry(rel);

    if (!path)
    {
        return false;
    }

    std::error_code ec;
    const bool      removed = std::filesystem::remove(*path, ec);

    if (ec)
    {
        Logger::error("file_tree: rm %s: %s", path->c_str(), ec.message().c_str());
        return false;
    }

    return removed;
}

std::optional<std::vector<std::string>> FileTree::list(const Path &rel) const
{
    const std::optional<std::vector<Entry>> found = this->entries(rel);

    if (!found)
    {
        return std::nullopt;
    }

    std::vector<std::string> names;
    names.reserve(found->size());

    for (const Entry &entry : *found)
    {
        names.push_back(entry.is_dir ? entry.name + "/" : entry.name);
    }

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

    std::optional<File> file = File::open(*path, File::Mode::Read);

    if (!file)
    {
        return {ReadStatus::Error, 0};
    }

    return file->read(offset, buf, std::min(max, FileTree::block_size));
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

    std::optional<File> file = File::open(*path, File::Mode::Append);

    if (!file)
    {
        return {WriteStatus::Error, 0};
    }

    return file->append(data, n);
}
