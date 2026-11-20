#include <cstdio>

#include "client_config.h"
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

    return 0;
}
