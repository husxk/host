#include "client_config.h"

#include "json_document.h"
#include "logger.h"

bool ClientConfig::load(const std::string &path)
{
    JsonDocument doc;

    if (!doc.parse_file(path) || !doc.is_object())
    {
        Logger::error("client_config: failed to parse %s", path.c_str());
        return false;
    }

    if (!doc.get_string(ClientConfig::socket_path_key, this->socket_path))
    {
        Logger::error("client_config: %s: missing or invalid \"%s\"",
                      path.c_str(),
                      ClientConfig::socket_path_key);
        return false;
    }

    Logger::info("client_config: loaded %s", path.c_str());
    Logger::info("client_config: socket path = %s", this->socket_path.c_str());

    return true;
}

const std::string &ClientConfig::get_socket_path() const
{
    return this->socket_path;
}
