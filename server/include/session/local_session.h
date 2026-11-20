#pragma once

#include <filesystem>
#include <optional>

class LocalSession
{
public:
    using Path = std::filesystem::path;

    static std::optional<LocalSession> open(const Path &socket_path);

    LocalSession(const LocalSession &)            = delete;
    LocalSession &operator=(const LocalSession &) = delete;
    LocalSession(LocalSession &&other) noexcept;
    LocalSession &operator=(LocalSession &&)      = delete;
    ~LocalSession();

    bool accept();
    void disconnect();
    bool connected() const;

private:
    LocalSession(int listen_fd, Path socket_path);

    int  listen_fd = -1;
    int  client_fd = -1;
    Path socket_path;
};
