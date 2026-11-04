#include "storage/file_tree.h"

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
