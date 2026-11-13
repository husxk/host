#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "command/command.h"
#include "storage/file_tree.h"

class CommandDispatcher
{
public:
    enum class Status
    {
        Ok,
        MoreData,
        Failed,
        Exit,
    };

    using Strings = std::vector<std::string>;
    using RawData = std::vector<char>;
    using Body    = std::variant<std::monostate, Strings, RawData, FileInfo, DiskInfo>;

    struct Result
    {
        Status status;
        Body   body {};
    };

    CommandDispatcher() = delete;
    explicit CommandDispatcher(std::shared_ptr<FileTree> tree);

    Result dispatch(const Command &cmd);

private:
    Result handle(const PrintCommand &cmd);
    Result handle(const LsCommand &cmd);
    Result handle(const ShowCommand &cmd);
    Result handle(const WriteCommand &cmd);
    Result handle(const StatCommand &cmd);
    Result handle(const MkdirCommand &cmd);
    Result handle(const MvCommand &cmd);
    Result handle(const RmCommand &cmd);
    Result handle(const DfCommand &cmd);
    Result handle(const ExitCommand &cmd);

    std::shared_ptr<FileTree> tree;
};
