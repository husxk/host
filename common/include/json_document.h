#pragma once

#include <string>
#include <vector>

#include <rapidjson/document.h>

class JsonDocument
{
public:
    JsonDocument()  = default;
    ~JsonDocument() = default;

    JsonDocument(const JsonDocument &)            = delete;
    JsonDocument &operator=(const JsonDocument &) = delete;

    JsonDocument(JsonDocument &&) = default;

    bool parse_file(const std::string &path);

    void reset_object();

    bool is_object() const;

    bool get_string(const char *key, std::string &out) const;
    bool get_int(const char *key, int &out) const;
    bool get_string_member(const char *key,
                           std::string &out,
                           bool &found) const;
    bool get_object_array(const char *key,
                          std::vector<JsonDocument> &out) const;

    void add(const char *key, const char *value);
    void add(const char *key, const std::string &value);
    void add(const char *key, unsigned value);
    void add(const char *key, const std::vector<JsonDocument> &items);

    bool write_pretty_file(const std::string &path) const;

private:
    bool parse_string(const std::string &text);

    static bool write_pretty_file(const std::string &path,
                                  const rapidjson::Document &document);

    rapidjson::Document doc;
    bool                parse_ok {false};
};
