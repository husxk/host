#include "json_document.h"

#include <climits>
#include <fstream>

#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

bool JsonDocument::parse_string(const std::string &text)
{
    this->parse_ok = false;

    if (text.empty())
    {
        return false;
    }

    this->doc.Parse(text.data(), text.size());

    if (this->doc.HasParseError())
    {
        return false;
    }

    this->parse_ok = true;
    return true;
}

void JsonDocument::reset_object()
{
    this->doc.SetObject();
    this->parse_ok = true;
}

void JsonDocument::add(const char *key, const char *value)
{
    auto &alloc = this->doc.GetAllocator();

    this->doc.AddMember(rapidjson::Value(key, alloc),
                        rapidjson::Value(value, alloc),
                        alloc);
}

void JsonDocument::add(const char *key, const std::string &value)
{
    auto &alloc = this->doc.GetAllocator();

    this->doc.AddMember(rapidjson::Value(key, alloc),
                        rapidjson::Value(value.c_str(), alloc),
                        alloc);
}

void JsonDocument::add(const char *key, unsigned value)
{
    auto &alloc = this->doc.GetAllocator();

    this->doc.AddMember(rapidjson::Value(key, alloc),
                        rapidjson::Value(value),
                        alloc);
}

void JsonDocument::add(const char *key, const std::vector<JsonDocument> &items)
{
    auto &alloc = this->doc.GetAllocator();

    rapidjson::Value array(rapidjson::kArrayType);

    for (const JsonDocument &item : items)
    {
        array.PushBack(rapidjson::Value(item.doc, alloc), alloc);
    }

    this->doc.AddMember(rapidjson::Value(key, alloc), array, alloc);
}

bool JsonDocument::write_pretty_file(const std::string &path) const
{
    if (!this->is_object())
    {
        return false;
    }

    return JsonDocument::write_pretty_file(path, this->doc);
}

bool JsonDocument::parse_file(const std::string &path)
{
    std::ifstream in(path, std::ios::binary);

    if (!in)
    {
        return false;
    }

    const std::string text{
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>()};

    return this->parse_string(text);
}

bool JsonDocument::is_object() const
{
    return this->parse_ok && this->doc.IsObject();
}

bool JsonDocument::get_string(const char *key, std::string &out) const
{
    if (!this->is_object() || key == nullptr)
    {
        return false;
    }

    const auto it = this->doc.FindMember(key);
    if (it == this->doc.MemberEnd() || !it->value.IsString())
    {
        return false;
    }

    out.assign(it->value.GetString(), it->value.GetStringLength());
    return true;
}

bool JsonDocument::get_int(const char *key, int &out) const
{
    if (!this->is_object() || key == nullptr)
    {
        return false;
    }

    const auto it = this->doc.FindMember(key);
    if (it == this->doc.MemberEnd())
    {
        return false;
    }

    const auto &v = it->value;

    if (v.IsInt())
    {
        out = v.GetInt();
        return true;
    }

    if (v.IsUint())
    {
        const unsigned u = v.GetUint();
        if (u > static_cast<unsigned>(INT_MAX))
        {
            return false;
        }

        out = static_cast<int>(u);
        return true;
    }

    return false;
}

bool JsonDocument::get_string_member(const char *key,
                                     std::string &out,
                                     bool &found) const
{
    found = false;
    out.clear();

    if (!this->is_object() || key == nullptr)
    {
        return false;
    }

    const auto it = this->doc.FindMember(key);
    if (it == this->doc.MemberEnd())
    {
        return true;
    }

    if (!it->value.IsString())
    {
        return false;
    }

    found = true;

    out.assign(it->value.GetString(), it->value.GetStringLength());
    return true;
}

bool JsonDocument::get_object_array(const char *key,
                                    std::vector<JsonDocument> &out) const
{
    out.clear();

    if (!this->is_object() || key == nullptr)
    {
        return false;
    }

    const auto it = this->doc.FindMember(key);
    if (it == this->doc.MemberEnd() || !it->value.IsArray())
    {
        return false;
    }

    for (const auto &item : it->value.GetArray())
    {
        if (!item.IsObject())
        {
            continue;
        }

        JsonDocument element;
        element.doc.CopyFrom(item, element.doc.GetAllocator());
        element.parse_ok = true;

        out.push_back(std::move(element));
    }

    return true;
}

bool JsonDocument::write_pretty_file(const std::string &path,
                                     const rapidjson::Document &document)
{
    rapidjson::StringBuffer                          buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    if (!document.Accept(writer))
    {
        return false;
    }

    std::ofstream out(path, std::ios::binary);
    if (!out)
    {
        return false;
    }

    out.write(buffer.GetString(), static_cast<std::streamsize>(buffer.GetSize()));
    return out.good();
}

