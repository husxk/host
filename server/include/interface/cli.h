#pragma once

#include <cstddef>
#include <cstdio>
#include <memory>
#include <string>

#include "command/command_dispatcher.h"

class Cli
{
public:
    Cli() = delete;
    explicit Cli(std::shared_ptr<CommandDispatcher> dispatcher,
                 FILE                              *in  = stdin,
                 FILE                              *out = stdout);

    void run();

    void print(const char *fmt, ...)
        __attribute__((format(printf, 2, 3)));

    void print_raw(const char *data, std::size_t n);

private:
    bool read_line(std::string &line);
    bool execute(const std::string &line);
    void print_help();

    std::shared_ptr<CommandDispatcher> dispatcher;
    FILE                              *in;
    FILE                              *out;

    static constexpr const char  prompt[]     = "host> ";
    static constexpr const char  help_name[]  = "help";
    static constexpr std::size_t line_reserve = 512;
};
