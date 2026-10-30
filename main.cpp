#include <cstdio>

#include "logger.h"

int main()
{
    if (!Logger::init())
    {
        std::fprintf(stderr, "host: failed to initialize logging\n");
        return 1;
    }

    Logger::info("host started");
    Logger::warn("this is a warning");
    Logger::error("this is an error");

    return 0;
}
