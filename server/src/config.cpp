#include "config.h"

#include "json_document.h"
#include "logger.h"

bool Config::load(const std::string &path)
{
    JsonDocument doc;

    if (!doc.parse_file(path) || !doc.is_object())
    {
        Logger::error("config: failed to parse %s", path.c_str());
        return false;
    }

    if (!doc.get_string(Config::root_path_key, this->root_path))
    {
        Logger::error("config: %s: missing or invalid \"%s\"",
                      path.c_str(),
                      Config::root_path_key);
        return false;
    }

    Logger::info("config: loaded %s", path.c_str());
    Logger::info("config: root fs path = %s", this->root_path.c_str());

    return true;
}

const std::string &Config::get_root_path() const
{
    return this->root_path;
}
