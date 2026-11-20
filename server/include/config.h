#pragma once

#include <string>

class Config
{
public:
    bool load(const std::string &path);

    const std::string &get_root_path() const;
    const std::string &get_socket_path() const;

private:
    std::string root_path;
    std::string socket_path;

    static constexpr const char root_path_key[]   = "root_path";
    static constexpr const char socket_path_key[] = "socket_path";
};
