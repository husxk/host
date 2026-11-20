#pragma once

#include <filesystem>
#include <optional>

class LocalClient
{
public:
    using Path = std::filesystem::path;

    static std::optional<LocalClient> connect(const Path &socket_path);

    LocalClient(const LocalClient &)            = delete;
    LocalClient &operator=(const LocalClient &) = delete;
    LocalClient(LocalClient &&other) noexcept;
    LocalClient &operator=(LocalClient &&)      = delete;
    ~LocalClient();

private:
    explicit LocalClient(int fd);

    int fd = -1;
};
