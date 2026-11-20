#include "local_client.h"

#include <cerrno>
#include <cstring>
#include <utility>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "logger.h"

std::optional<LocalClient> LocalClient::connect(const Path &socket_path)
{
    sockaddr_un addr {};

    if (socket_path.native().size() >= sizeof(addr.sun_path))
    {
        Logger::error("local_client: socket path too long: %s", socket_path.c_str());
        return std::nullopt;
    }

    addr.sun_family = AF_UNIX;
    std::memcpy(addr.sun_path, socket_path.c_str(), socket_path.native().size());

    const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);

    if (fd < 0)
    {
        Logger::error("local_client: socket: %s", std::strerror(errno));
        return std::nullopt;
    }

    LocalClient client(fd);

    if (::connect(fd, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) != 0)
    {
        Logger::error("local_client: connect %s: %s", socket_path.c_str(), std::strerror(errno));
        return std::nullopt;
    }

    Logger::info("local_client: connected to %s", socket_path.c_str());

    return client;
}

LocalClient::LocalClient(int fd)
    : fd(fd)
{
}

LocalClient::LocalClient(LocalClient &&other) noexcept
    : fd(std::exchange(other.fd, -1))
{
}

LocalClient::~LocalClient()
{
    if (this->fd >= 0)
    {
        ::close(this->fd);
    }
}
