#pragma once

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

struct ExitCommand
{
};

using Command = std::variant<ScanCommand, PrintCommand, LsCommand, ExitCommand>;
