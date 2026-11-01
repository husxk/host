#include <cstdio>

#include "logger.h"
#include "storage/file_tree.h"

int main()
{
    if (!Logger::init())
    {
        std::fprintf(stderr, "host: failed to initialize logging\n");
        return 1;
    }

    FileTree tree("build");

    if (tree.scan())
    {
        tree.print();
    }

    return 0;
}
