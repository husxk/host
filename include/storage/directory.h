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
    std::vector<std::string> list() const;

    const Directory *find(const Path &rel) const;
    Directory       *find(const Path &rel);
    const File      *find_file(const Path &rel) const;

    void add_file(const std::string &name);

    const std::string &get_name() const;

private:
    void print(int depth) const;

    std::string            name;
    std::vector<File>      files;
    std::vector<Directory> dirs;
};
