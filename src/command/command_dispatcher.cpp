#include "command/command_dispatcher.h"

#include <cassert>
#include <utility>

CommandDispatcher::CommandDispatcher(std::shared_ptr<FileTree> tree)
    : tree(std::move(tree))
{
    assert(this->tree);
}

CommandDispatcher::Result CommandDispatcher::dispatch(const Command &cmd)
{
    return std::visit([this](const auto &c) { return this->handle(c); }, cmd);
}

CommandDispatcher::Result CommandDispatcher::handle(const PrintCommand &)
{
    auto lines = this->tree->print();

    if (!lines)
    {
        return {Status::Failed};
    }

    return {Status::Ok, std::move(*lines)};
}

CommandDispatcher::Result CommandDispatcher::handle(const LsCommand &cmd)
{
    auto names = this->tree->list(cmd.path);

    if (!names)
    {
        return {Status::Failed};
    }

    return {Status::Ok, std::move(*names)};
}

CommandDispatcher::Result CommandDispatcher::handle(const ShowCommand &cmd)
{
    RawData data(FileTree::block_size);

    const FileTree::ReadResult read = this->tree->read(cmd.path, cmd.offset, data.data(), data.size());

    if (read.status == FileTree::ReadStatus::Error)
    {
        return {Status::Failed};
    }

    data.resize(read.n);

    const Status status = read.status == FileTree::ReadStatus::MoreData ? Status::MoreData : Status::Ok;

    return {status, std::move(data)};
}

CommandDispatcher::Result CommandDispatcher::handle(const WriteCommand &cmd)
{
    const FileTree::WriteResult write = this->tree->write(cmd.path, cmd.data.data(), cmd.data.size());

    return {write.status == FileTree::WriteStatus::Ok ? Status::Ok : Status::Failed};
}

CommandDispatcher::Result CommandDispatcher::handle(const StatCommand &cmd)
{
    const std::optional<FileInfo> info = this->tree->stat(cmd.path);

    if (!info)
    {
        return {Status::Failed};
    }

    return {Status::Ok, *info};
}

CommandDispatcher::Result CommandDispatcher::handle(const MkdirCommand &cmd)
{
    return {this->tree->mkdir(cmd.path) ? Status::Ok : Status::Failed};
}

CommandDispatcher::Result CommandDispatcher::handle(const MvCommand &cmd)
{
    return {this->tree->mv(cmd.src, cmd.dst) ? Status::Ok : Status::Failed};
}

CommandDispatcher::Result CommandDispatcher::handle(const RmCommand &cmd)
{
    return {this->tree->rm(cmd.path) ? Status::Ok : Status::Failed};
}

CommandDispatcher::Result CommandDispatcher::handle(const DfCommand &)
{
    const std::optional<DiskInfo> info = this->tree->df();

    if (!info)
    {
        return {Status::Failed};
    }

    return {Status::Ok, *info};
}

CommandDispatcher::Result CommandDispatcher::handle(const ExitCommand &)
{
    return {Status::Exit};
}
