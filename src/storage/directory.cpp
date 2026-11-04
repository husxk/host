#include "storage/directory.h"

#include <algorithm>
#include <system_error>
#include <utility>

#include "logger.h"

Directory::Directory(const std::string &name)
    : name(name)
{
}

bool Directory::scan(const Path &path)
{
    this->files.clear();
    this->dirs.clear();

    std::error_code ec;

    std::filesystem::directory_iterator       it(path, ec);
    const std::filesystem::directory_iterator end;

    for (; !ec && it != end; it.increment(ec))
    {
        const std::filesystem::file_status status = it->symlink_status(ec);

        if (ec)
        {
            Logger::error("directory: stat %s: %s",
                          it->path().c_str(),
                          ec.message().c_str());
            continue;
        }

        const std::string entry_name = it->path().filename().string();

        if (std::filesystem::is_regular_file(status))
        {
            this->files.emplace_back(entry_name);
        }
        else if (std::filesystem::is_directory(status))
        {
            Directory sub(entry_name);

            if (sub.scan(it->path()))
            {
                this->dirs.push_back(std::move(sub));
            }
        }
    }

    if (ec)
    {
        Logger::error("directory: scan %s: %s", path.c_str(), ec.message().c_str());
        return false;
    }

    return true;
}

void Directory::print() const
{
    this->print(0);
}

void Directory::print(int depth) const
{
    Logger::plain("%*s%s/", depth * 2, "", this->name.c_str());

    for (const File &file : this->files)
    {
        Logger::plain("%*s%s", (depth + 1) * 2, "", file.get_name().c_str());
    }

    for (const Directory &dir : this->dirs)
    {
        dir.print(depth + 1);
    }
}

std::vector<std::string> Directory::list() const
{
    std::vector<std::string> names;
    names.reserve(this->files.size() + this->dirs.size());

    for (const File &file : this->files)
    {
        names.push_back(file.get_name());
    }

    for (const Directory &dir : this->dirs)
    {
        names.push_back(dir.name + "/");
    }

    std::sort(names.begin(), names.end());

    return names;
}

const Directory *Directory::find(const Path &rel) const
{
    const Directory *current = this;

    for (const Path &part : rel)
    {
        if (part.empty() || part == "." || part == "/")
        {
            continue;
        }

        if (part == "..")
        {
            return nullptr;
        }

        const auto it = std::find_if(current->dirs.begin(),
                                     current->dirs.end(),
                                     [&part](const Directory &dir) { return dir.name == part; });

        if (it == current->dirs.end())
        {
            return nullptr;
        }

        current = &*it;
    }

    return current;
}

const std::string &Directory::get_name() const
{
    return this->name;
}
