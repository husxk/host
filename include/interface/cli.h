#pragma once

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

class Cli
{
public:
    struct Command
    {
        std::string              name;
        std::vector<std::string> args;
    };

    explicit Cli(FILE *in = stdin, FILE *out = stdout);

    bool read_command(Command &out);

    void print(const char *fmt, ...)
        __attribute__((format(printf, 2, 3)));

private:
    bool read_line(std::string &line);

    static void tokenize(const std::string &line, Command &out);

    FILE *in;
    FILE *out;

    static constexpr const char  prompt[]     = "host> ";
    static constexpr std::size_t line_reserve = 512;
};
