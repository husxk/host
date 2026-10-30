#include "storage/file.h"

File::File(const std::string &name)
    : name(name)
{
}

const std::string &File::get_name() const
{
    return this->name;
}
