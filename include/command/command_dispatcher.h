#pragma once

#include <memory>

#include "command/command.h"
#include "storage/file_tree.h"

class CommandDispatcher
{
public:
    enum class Result
    {
        Ok,
        Failed,
        Exit,
    };

    CommandDispatcher() = delete;
    explicit CommandDispatcher(std::shared_ptr<FileTree> tree);

    Result dispatch(const Command &cmd);

private:
    Result handle(const ScanCommand &cmd);
    Result handle(const PrintCommand &cmd);
    Result handle(const ExitCommand &cmd);

    std::shared_ptr<FileTree> tree;
};
