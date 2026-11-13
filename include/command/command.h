#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

struct PrintCommand
{
};

struct LsCommand
{
    std::string path;
};

struct ShowCommand
{
    std::string   path;
    std::uint64_t offset = 0;
};

struct WriteCommand
{
    std::string       path;
    std::vector<char> data;
};

struct StatCommand
{
    std::string path;
};

struct MkdirCommand
{
    std::string path;
};

struct MvCommand
{
    std::string src;
    std::string dst;
};

struct RmCommand
{
    std::string path;
};

struct DfCommand
{
};

struct ExitCommand
{
};

using Command = std::variant<PrintCommand,
                             LsCommand,
                             ShowCommand,
                             WriteCommand,
                             StatCommand,
                             MkdirCommand,
                             MvCommand,
                             RmCommand,
                             DfCommand,
                             ExitCommand>;
