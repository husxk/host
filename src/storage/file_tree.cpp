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

std::optional<std::vector<std::string>> FileTree::list(const Path &rel) const
{
    const Directory *dir = this->root.find(rel);

    if (dir == nullptr)
    {
        return std::nullopt;
    }

    return dir->list();
}

FileTree::ReadResult FileTree::read(const Path    &rel,
                                    std::uint64_t  offset,
                                    char          *buf,
                                    std::size_t    max)
{
    if (this->root.find_file(rel) == nullptr)
    {
        return {ReadStatus::Error, 0};
    }

    const Path    path = this->root_path / rel.relative_path();
    std::ifstream in(path, std::ios::binary);

    if (!in)
    {
        Logger::error("file_tree: open %s failed", path.c_str());
        return {ReadStatus::Error, 0};
    }

    const std::size_t count = std::min(max, FileTree::block_size);

    in.seekg(static_cast<std::streamoff>(offset));
    in.read(buf, static_cast<std::streamsize>(count));

    if (in.bad())
    {
        Logger::error("file_tree: read %s failed", path.c_str());
        return {ReadStatus::Error, 0};
    }

    const std::size_t n   = static_cast<std::size_t>(in.gcount());
    const bool        eof = in.peek() == std::ifstream::traits_type::eof();

    return {eof ? ReadStatus::Ok : ReadStatus::MoreData, n};
}

FileTree::WriteResult FileTree::write(const Path &rel, const char *data, std::size_t n)
{
    const std::string name = rel.filename().string();

    if (name.empty() || name == "." || name == "..")
    {
        return {WriteStatus::Error, 0};
    }

    Directory *parent = this->root.find(rel.parent_path());

    if (parent == nullptr || parent->find(name) != nullptr)
    {
        return {WriteStatus::Error, 0};
    }

    const bool exists = parent->find_file(name) != nullptr;

    const Path    path = this->root_path / rel.relative_path();
    std::ofstream out(path, std::ios::binary | std::ios::app);

    if (!out)
    {
        Logger::error("file_tree: open %s failed", path.c_str());
        return {WriteStatus::Error, 0};
    }

    out.write(data, static_cast<std::streamsize>(n));
    out.flush();

    if (!out)
    {
        Logger::error("file_tree: write %s failed", path.c_str());
        return {WriteStatus::Error, 0};
    }

    if (!exists)
    {
        parent->add_file(name);
    }

    return {WriteStatus::Ok, n};
}
