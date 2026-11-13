#include "interface/cli.h"

#include <cassert>
#include <chrono>
#include <cstdarg>
#include <format>
#include <optional>
#include <sstream>
#include <utility>
#include <variant>
#include <vector>

#include "command/command.h"

namespace
{

using Args = std::vector<std::string>;

template <typename T>
std::optional<Command> make_no_args(const Args &args)
{
    if (!args.empty())
    {
        return std::nullopt;
    }

    return T {};
}

template <typename T>
std::optional<Command> make_one_arg(const Args &args)
{
    if (args.size() != 1)
    {
        return std::nullopt;
    }

    return T {args[0]};
}

std::optional<Command> make_ls(const Args &args)
{
    if (args.size() > 1)
    {
        return std::nullopt;
    }

    return LsCommand {args.empty() ? "" : args[0]};
}

std::optional<Command> make_write(const Args &args)
{
    if (args.size() != 2)
    {
        return std::nullopt;
    }

    std::vector<char> data(args[1].begin(), args[1].end());

    if (!data.empty())
    {
        data.push_back('\n');
    }

    return WriteCommand {args[0], std::move(data)};
}

std::optional<Command> make_mv(const Args &args)
{
    if (args.size() != 2)
    {
        return std::nullopt;
    }

    return MvCommand {args[0], args[1]};
}

bool handle_none(Cli &, Command &, const CommandDispatcher::Result &)
{
    return true;
}

bool handle_strings(Cli &cli, Command &, const CommandDispatcher::Result &result)
{
    if (const auto *strings = std::get_if<CommandDispatcher::Strings>(&result.body))
    {
        for (const std::string &line : *strings)
        {
            cli.print("%s", line.c_str());
        }
    }

    return true;
}

bool handle_show(Cli &cli, Command &command, const CommandDispatcher::Result &result)
{
    const auto *raw = std::get_if<CommandDispatcher::RawData>(&result.body);

    if (raw == nullptr)
    {
        return true;
    }

    cli.print_raw(raw->data(), raw->size());

    if (result.status == CommandDispatcher::Status::MoreData && !raw->empty())
    {
        std::get<ShowCommand>(command).offset += raw->size();
        return false;
    }

    cli.print_raw("\n", 1);
    return true;
}

bool handle_stat(Cli &cli, Command &, const CommandDispatcher::Result &result)
{
    const auto *info = std::get_if<FileInfo>(&result.body);

    if (info == nullptr)
    {
        return true;
    }

    const std::string mtime = std::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::floor<std::chrono::seconds>(info->mtime));

    cli.print("type:  %s", info->type == FileInfo::Type::File ? "file" : "dir");
    cli.print("size:  %llu", static_cast<unsigned long long>(info->size));
    cli.print("mtime: %s UTC", mtime.c_str());

    return true;
}

struct Entry
{
    const char *name;
    const char *usage;
    const char *description;
    std::optional<Command> (*make)(const Args &args);
    bool (*handle_result)(Cli &cli, Command &command, const CommandDispatcher::Result &result);
    bool raw_rest;
};

constexpr Entry entries[] = {
    {"print", "print", "print the file tree", make_no_args<PrintCommand>, handle_strings, false},
    {"ls", "ls [dir]", "list directory contents", make_ls, handle_strings, false},
    {"show", "show <file>", "print file contents", make_one_arg<ShowCommand>, handle_show, false},
    {"write", "write <file> [text]", "append text to file", make_write, handle_none, true},
    {"stat", "stat <path>", "show file info", make_one_arg<StatCommand>, handle_stat, false},
    {"mkdir", "mkdir <dir>", "create directory", make_one_arg<MkdirCommand>, handle_none, false},
    {"mv", "mv <src> <dst>", "move or rename", make_mv, handle_none, false},
    {"rm", "rm <path>", "remove file or empty directory", make_one_arg<RmCommand>, handle_none, false},
    {"exit", "exit", "quit", make_no_args<ExitCommand>, handle_none, false},
};

const Entry *find_entry(const std::string &name)
{
    for (const Entry &entry : entries)
    {
        if (name == entry.name)
        {
            return &entry;
        }
    }

    return nullptr;
}

} // namespace

Cli::Cli(std::shared_ptr<CommandDispatcher> dispatcher, FILE *in, FILE *out)
    : dispatcher(std::move(dispatcher))
    , in(in)
    , out(out)
{
    assert(this->dispatcher);
}

void Cli::run()
{
    std::string line;
    line.reserve(Cli::line_reserve);

    do
    {
        std::fputs(Cli::prompt, this->out);
        std::fflush(this->out);

        if (!this->read_line(line))
        {
            std::fputc('\n', this->out);
            return;
        }
    } while (this->execute(line));
}

void Cli::print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    std::vfprintf(this->out, fmt, args);

    va_end(args);

    std::fputc('\n', this->out);
    std::fflush(this->out);
}

void Cli::print_raw(const char *data, std::size_t n)
{
    std::fwrite(data, 1, n, this->out);
    std::fflush(this->out);
}

bool Cli::read_line(std::string &line)
{
    line.clear();

    int c;

    while ((c = std::fgetc(this->in)) != EOF)
    {
        if (c == '\n')
        {
            return true;
        }

        line.push_back(static_cast<char>(c));
    }

    return !line.empty();
}

bool Cli::execute(const std::string &line)
{
    std::istringstream stream(line);
    std::string        name;
    std::string        arg;
    Args               args;

    if (!(stream >> name))
    {
        return true;
    }

    const Entry *entry = find_entry(name);

    if (entry != nullptr && entry->raw_rest)
    {
        if (stream >> arg)
        {
            std::string rest;

            // Skip the single separator after the path; the rest is raw data.
            stream.get();
            std::getline(stream, rest);

            args.push_back(arg);
            args.push_back(rest);
        }
    }
    else
    {
        while (stream >> arg)
        {
            args.push_back(arg);
        }
    }

    if (name == Cli::help_name)
    {
        if (!args.empty())
        {
            this->print("usage: %s", Cli::help_name);
            return true;
        }

        this->print_help();
        return true;
    }

    if (entry == nullptr)
    {
        this->print("unknown command: %s", name.c_str());
        return true;
    }

    std::optional<Command> command = entry->make(args);

    if (!command)
    {
        this->print("usage: %s", entry->usage);
        return true;
    }

    CommandDispatcher::Result result;

    do
    {
        result = this->dispatcher->dispatch(*command);
    } while (!entry->handle_result(*this, *command, result));

    switch (result.status)
    {
    case CommandDispatcher::Status::Ok:
        return true;

    case CommandDispatcher::Status::Failed:
        this->print("%s failed", name.c_str());
        return true;

    case CommandDispatcher::Status::Exit:
        return false;

    default:
        this->print("%s: unexpected state", name.c_str());
        return true;
    }

    return true;
}

void Cli::print_help()
{
    this->print("commands:");

    for (const Entry &entry : entries)
    {
        this->print("  %-20s %s", entry.usage, entry.description);
    }

    this->print("  %-20s %s", Cli::help_name, "show this help");
}
