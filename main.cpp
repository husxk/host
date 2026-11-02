#include <cstdio>

#include "config.h"
#include "interface/cli.h"
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

    FileTree     tree(config.get_root_path());
    Cli          cli;
    Cli::Command cmd;

    while (cli.read_command(cmd))
    {
        if (cmd.name == "exit")
        {
            break;
        }
        else if (cmd.name == "help")
        {
            cli.print("commands:");
            cli.print("  scan   rescan the file tree");
            cli.print("  print  print the file tree");
            cli.print("  help   show this help");
            cli.print("  exit   quit");
        }
        else if (cmd.name == "scan")
        {
            if (!tree.scan())
            {
                cli.print("scan failed");
            }
        }
        else if (cmd.name == "print")
        {
            tree.print();
        }
        else
        {
            cli.print("unknown command: %s", cmd.name.c_str());
        }
    }

    return 0;
}
