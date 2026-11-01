#include <cstdio>

#include "config.h"
#include "logger.h"
#include "storage/file_tree.h"

int main()
{
    if (!Logger::init())
    {
        std::fprintf(stderr, "host: failed to initialize logging\n");
        return 1;
    }

    Config config;

    if (!config.load("./config.json"))
    {
        return 1;
    }

    FileTree tree(config.get_root_path());

    if (tree.scan())
    {
        tree.print();
    }

    return 0;
}
