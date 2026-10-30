#pragma once

#include <string>

class File
{
public:
    explicit File(const std::string &name);

    const std::string &get_name() const;

private:
    std::string name;
};
