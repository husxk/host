#include "session/local_session.h"

#include <cerrno>
#include <cstring>
#include <system_error>
#include <utility>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "logger.h"

namespace
{

constexpr mode_t socket_mode = 0600;

bool make_address(const std::filesystem::path &path, sockaddr_un &addr)
{
    if (path.native().size() >= sizeof(addr.sun_path))
    {
        Logger::error("local_session: socket path too long: %s", path.c_str());
        return false;
    }

    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::memcpy(addr.sun_path, path.c_str(), path.native().size());

    return true;
}

bool remove_stale_socket(const std::filesystem::path &path, const sockaddr_un &addr)
{
    std::error_code                    ec;
    const std::filesystem::file_status status = std::filesystem::symlink_status(path, ec);

    if (!std::filesystem::exists(status))
    {
        return true;
    }

    if (!std::filesystem::is_socket(status))
    {
        Logger::error("local_session: %s exists and is not a socket", path.c_str());
        return false;
    }

    const int  fd        = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    const bool connected = fd >= 0 && ::connect(fd, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) == 0;
    const int  error     = errno;

    if (fd >= 0)
    {
        ::close(fd);
    }

    if (connected)
    {
        Logger::error("local_session: %s is in use by another server", path.c_str());
        return false;
    }

    if (error != ECONNREFUSED)
    {
        Logger::error("local_session: probe %s: %s", path.c_str(), std::strerror(error));
        return false;
    }

    return std::filesystem::remove(path, ec);
}

} // namespace

std::optional<LocalSession> LocalSession::open(const Path &socket_path)
{
    sockaddr_un addr;

    if (!make_address(socket_path, addr) || !remove_stale_socket(socket_path, addr))
    {
        return std::nullopt;
    }

    const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);

    if (fd < 0)
    {
        Logger::error("local_session: socket: %s", std::strerror(errno));
        return std::nullopt;
    }

    LocalSession session(fd, socket_path);

    if (::bind(fd, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) != 0)
    {
        Logger::error("local_session: bind %s: %s", socket_path.c_str(), std::strerror(errno));
        session.socket_path.clear();
        return std::nullopt;
    }

    if (::chmod(socket_path.c_str(), socket_mode) != 0 || ::listen(fd, SOMAXCONN) != 0)
    {
        Logger::error("local_session: listen %s: %s", socket_path.c_str(), std::strerror(errno));
        return std::nullopt;
    }

    return session;
}

LocalSession::LocalSession(int listen_fd, Path socket_path)
    : listen_fd(listen_fd)
    , socket_path(std::move(socket_path))
{
}

LocalSession::LocalSession(LocalSession &&other) noexcept
    : listen_fd(std::exchange(other.listen_fd, -1))
    , client_fd(std::exchange(other.client_fd, -1))
    , socket_path(std::move(other.socket_path))
{
    other.socket_path.clear();
}

LocalSession::~LocalSession()
{
    this->disconnect();

    if (this->listen_fd >= 0)
    {
        ::close(this->listen_fd);
    }

    if (!this->socket_path.empty())
    {
        ::unlink(this->socket_path.c_str());
    }
}

bool LocalSession::accept()
{
    if (this->connected())
    {
        return false;
    }

    const int fd = ::accept4(this->listen_fd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (fd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            Logger::error("local_session: accept: %s", std::strerror(errno));
        }

        return false;
    }

    this->client_fd = fd;

    return true;
}

void LocalSession::disconnect()
{
    if (this->client_fd >= 0)
    {
        ::close(this->client_fd);
        this->client_fd = -1;
    }
}

bool LocalSession::connected() const
{
    return this->client_fd >= 0;
}
