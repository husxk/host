#pragma once

#include <filesystem>

#include "storage/directory.h"

class FileTree
{
public:
    using Path = std::filesystem::path;

    explicit FileTree(const Path &root_path);

    bool scan();
    void print() const;

private:
    Path      root_path;
    Directory root;
};
