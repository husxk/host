#include <cstdio>
#include <memory>
#include <optional>

#include "command/command_dispatcher.h"
#include "config.h"
#include "interface/cli.h"
#include "logger.h"
#include "session/local_session.h"
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

    std::optional<LocalSession> session = LocalSession::open(config.get_socket_path());

    if (!session)
    {
        return 1;
    }

    auto tree       = std::make_shared<FileTree>(config.get_root_path());
    auto dispatcher = std::make_shared<CommandDispatcher>(tree);
    Cli  cli(dispatcher);

// cli.run();

    if (!session->wait_and_accept())
    {
        return 1;
    }

    return 0;
}
