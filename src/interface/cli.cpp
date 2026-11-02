#include "interface/cli.h"

#include <cstdarg>
#include <sstream>

Cli::Cli(FILE *in, FILE *out)
    : in(in)
    , out(out)
{
}

bool Cli::read_command(Command &out)
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
            return false;
        }

        Cli::tokenize(line, out);
    } while (out.name.empty());

    return true;
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

void Cli::tokenize(const std::string &line, Command &out)
{
    out.name.clear();
    out.args.clear();

    std::istringstream stream(line);

    if (!(stream >> out.name))
    {
        return;
    }

    std::string arg;

    while (stream >> arg)
    {
        out.args.push_back(arg);
    }
}
