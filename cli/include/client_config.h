#pragma once

#include <string>

class ClientConfig
{
public:
    bool load(const std::string &path);

    const std::string &get_socket_path() const;

private:
    std::string socket_path;

    static constexpr const char socket_path_key[] = "socket_path";
};
