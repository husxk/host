#include "interface/cli.h"

#include <cassert>
#include <cstdarg>
#include <optional>
#include <sstream>
#include <utility>
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

std::optional<Command> make_ls(const Args &args)
{
    if (args.size() > 1)
    {
        return std::nullopt;
    }

    return LsCommand {args.empty() ? "" : args[0]};
}

struct Entry
{
    const char *name;
    const char *usage;
    const char *description;
    std::optional<Command> (*make)(const Args &args);
};

constexpr Entry entries[] = {
    {"scan", "scan", "rescan the file tree", make_no_args<ScanCommand>},
    {"print", "print", "print the file tree", make_no_args<PrintCommand>},
    {"ls", "ls [dir]", "list directory contents", make_ls},
    {"exit", "exit", "quit", make_no_args<ExitCommand>},
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

    while (stream >> arg)
    {
        args.push_back(arg);
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

    const Entry *entry = find_entry(name);

    if (entry == nullptr)
    {
        this->print("unknown command: %s", name.c_str());
        return true;
    }

    const std::optional<Command> command = entry->make(args);

    if (!command)
    {
        this->print("usage: %s", entry->usage);
        return true;
    }

    const CommandDispatcher::Result result = this->dispatcher->dispatch(*command);

    for (const std::string &result_line : result.lines)
    {
        this->print("%s", result_line.c_str());
    }

    switch (result.status)
    {
    case CommandDispatcher::Status::Ok:
        return true;

    case CommandDispatcher::Status::Failed:
        this->print("%s failed", name.c_str());
        return true;

    case CommandDispatcher::Status::Exit:
        return false;
    }

    return true;
}

void Cli::print_help()
{
    this->print("commands:");

    for (const Entry &entry : entries)
    {
        this->print("  %-10s %s", entry.usage, entry.description);
    }

    this->print("  %-10s %s", Cli::help_name, "show this help");
}
