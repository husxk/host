#pragma once

#include <string>

class Config
{
public:
    bool load(const std::string &path);

    const std::string &get_root_path() const;

private:
    std::string root_path;

    static constexpr const char root_path_key[] = "root_path";
};
