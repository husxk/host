#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "storage/directory.h"

class FileTree
{
public:
    using Path = std::filesystem::path;

    explicit FileTree(const Path &root_path);

    bool scan();
    void print() const;
    std::optional<std::vector<std::string>> list(const Path &rel) const;

private:
    Path      root_path;
    Directory root;
};
