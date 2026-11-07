#pragma once

#include <cstdint>
#include <string>
#include <variant>

struct ScanCommand
{
};

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

struct ExitCommand
{
};

using Command = std::variant<ScanCommand, PrintCommand, LsCommand, ShowCommand, ExitCommand>;
