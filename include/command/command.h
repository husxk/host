#pragma once

#include <variant>

struct ScanCommand
{
};

struct PrintCommand
{
};

struct ExitCommand
{
};

using Command = std::variant<ScanCommand, PrintCommand, ExitCommand>;
