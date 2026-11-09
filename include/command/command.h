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

struct ExitCommand
{
};

using Command = std::variant<PrintCommand, LsCommand, ShowCommand, WriteCommand, ExitCommand>;
