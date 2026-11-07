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

CommandDispatcher::Result CommandDispatcher::handle(const ScanCommand &)
{
    return {this->tree->scan() ? Status::Ok : Status::Failed};
}

CommandDispatcher::Result CommandDispatcher::handle(const PrintCommand &)
{
    this->tree->print();
    return {Status::Ok};
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

CommandDispatcher::Result CommandDispatcher::handle(const ExitCommand &)
{
    return {Status::Exit};
}
