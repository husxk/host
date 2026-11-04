#pragma once

#include <memory>
#include <string>
#include <vector>

#include "command/command.h"
#include "storage/file_tree.h"

class CommandDispatcher
{
public:
    enum class Status
    {
        Ok,
        Failed,
        Exit,
    };

    struct Result
    {
        Status                   status;
        std::vector<std::string> lines {};
    };

    CommandDispatcher() = delete;
    explicit CommandDispatcher(std::shared_ptr<FileTree> tree);

    Result dispatch(const Command &cmd);

private:
    Result handle(const ScanCommand &cmd);
    Result handle(const PrintCommand &cmd);
    Result handle(const LsCommand &cmd);
    Result handle(const ExitCommand &cmd);

    std::shared_ptr<FileTree> tree;
};
