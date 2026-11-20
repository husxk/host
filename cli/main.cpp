#include <cstdio>
#include <optional>

#include "client_config.h"
#include "local_client.h"
#include "logger.h"

int main()
{
    if (!Logger::init())
    {
        std::fprintf(stderr, "host-cli: failed to initialize logging\n");
        return 1;
    }

    ClientConfig config;

    if (!config.load("./cli.json"))
    {
        return 1;
    }

    std::optional<LocalClient> client = LocalClient::connect(config.get_socket_path());

    if (!client)
    {
        return 1;
    }

    return 0;
}
