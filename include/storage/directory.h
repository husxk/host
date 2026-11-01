#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "storage/file.h"

class Directory
{
public:
    using Path = std::filesystem::path;

    explicit Directory(const std::string &name);

    bool scan(const Path &path);
    void print() const;

    const std::string &get_name() const;

private:
    void print(int depth) const;

    std::string            name;
    std::vector<File>      files;
    std::vector<Directory> dirs;
};
