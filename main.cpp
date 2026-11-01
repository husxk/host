#include <cstdio>

#include "logger.h"
#include "storage/directory.h"

int main()
{
    if (!Logger::init())
    {
        std::fprintf(stderr, "host: failed to initialize logging\n");
        return 1;
    }

    Directory root("build/");

    if (root.scan("build"))
    {
        root.print();
    }

    return 0;
}
