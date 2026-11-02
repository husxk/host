#include "interface/cli.h"

#include <cassert>
#include <cstdarg>
#include <sstream>
#include <utility>

#include "command/command.h"

namespace
{

struct Entry
{
    const char *name;
    const char *description;
    Command     command;
};

constexpr Entry entries[] = {
    {"scan", "rescan the file tree", ScanCommand {}},
    {"print", "print the file tree", PrintCommand {}},
    {"exit", "quit", ExitCommand {}},
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
    std::string        extra;

    if (!(stream >> name))
    {
        return true;
    }

    if (stream >> extra)
    {
        this->print("%s: unexpected arguments", name.c_str());
        return true;
    }

    if (name == Cli::help_name)
    {
        this->print_help();
        return true;
    }

    const Entry *entry = find_entry(name);

    if (entry == nullptr)
    {
        this->print("unknown command: %s", name.c_str());
        return true;
    }

    switch (this->dispatcher->dispatch(entry->command))
    {
    case CommandDispatcher::Result::Ok:
        return true;

    case CommandDispatcher::Result::Failed:
        this->print("%s failed", name.c_str());
        return true;

    case CommandDispatcher::Result::Exit:
        return false;
    }

    return true;
}

void Cli::print_help()
{
    this->print("commands:");

    for (const Entry &entry : entries)
    {
        this->print("  %-6s %s", entry.name, entry.description);
    }

    this->print("  %-6s %s", Cli::help_name, "show this help");
}
